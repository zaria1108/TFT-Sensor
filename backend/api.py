from queue import Empty

from bson import ObjectId
from flask import Blueprint, Response, jsonify, request, stream_with_context

from database import build_document, collection, serialize
import mqtt_handler
from realtime import encode_sse, publish, subscribe, unsubscribe

api_bp = Blueprint("api", __name__, url_prefix="/api")
STREAM_SNAPSHOT_INTERVAL = 3

SENSOR_FIELDS = [
    "temperature",
    "humidity",
    "bmp_temperature",
    "pressure",
    "soil_moisture",
]


def _get_latest_reading():
    reading = collection.find_one({}, sort=[("timestamp", -1)])
    return serialize(reading) if reading else None


def _get_recent_readings(limit: int = 100):
    projection = {"_id": 1, "timestamp": 1, **{field: 1 for field in SENSOR_FIELDS}}
    readings = list(
        collection.find({}, projection)
        .sort("timestamp", -1)
        .limit(limit)
    )
    return [serialize(reading) for reading in readings]


def _get_count() -> int:
    return collection.count_documents({})


def _mqtt_payload():
    return {
        "connected": mqtt_handler.status["connected"],
        "last_message_at": mqtt_handler.status["last_message_at"],
        "messages_total": mqtt_handler.status["messages_total"],
        "last_error": mqtt_handler.status["last_error"],
    }


def build_realtime_snapshot():
    return {
        "latestReading": _get_latest_reading(),
        "readings": _get_recent_readings(),
        "count": _get_count(),
        "mqtt": _mqtt_payload(),
    }


@api_bp.post("/create")
def create_reading():
    try:
        data = request.get_json() or {}
        document = build_document(data)
        result = collection.insert_one(document)
        created = serialize({**document, "_id": result.inserted_id})
        publish("reading_created", {"reading": created, "count": _get_count()})
        return jsonify({"success": True, "id": str(result.inserted_id)}), 201
    except Exception as exc:
        return jsonify({"error": str(exc)}), 400


@api_bp.get("/latest")
def get_latest():
    reading = _get_latest_reading()
    if reading:
        return jsonify(reading)
    return jsonify({"error": "No data yet"}), 404


@api_bp.get("/data")
def get_all_data():
    return jsonify(_get_recent_readings())


@api_bp.get("/count")
def get_count():
    count = _get_count()
    return jsonify({"count": count, "total_readings": count})


@api_bp.get("/status")
def get_status():
    try:
        collection.database.client.admin.command("ping")
        mongo_ok = True
        mongo_err = None
    except Exception as exc:
        mongo_ok = False
        mongo_err = str(exc)

    return jsonify(
        {
            "mqtt": _mqtt_payload(),
            "mongodb": {
                "ok": mongo_ok,
                "error": mongo_err,
                "total_readings": _get_count() if mongo_ok else None,
            },
        }
    )


@api_bp.get("/mqtt/status")
def get_mqtt_status():
    return jsonify(_mqtt_payload())


@api_bp.get("/stream")
def stream_updates():
    queue = subscribe()

    @stream_with_context
    def event_stream():
        try:
            yield encode_sse("snapshot", build_realtime_snapshot())
            while True:
                try:
                    message = queue.get(timeout=STREAM_SNAPSHOT_INTERVAL)
                    yield encode_sse(message["event"], message["data"])
                except Empty:
                    yield encode_sse("snapshot", build_realtime_snapshot())
        finally:
            unsubscribe(queue)

    return Response(
        event_stream(),
        mimetype="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "X-Accel-Buffering": "no",
        },
    )


@api_bp.put("/update/<doc_id>")
def update_reading(doc_id):
    try:
        data = request.get_json() or {}
        update_fields = {field: data[field] for field in SENSOR_FIELDS if field in data}

        if not update_fields:
            return jsonify({"error": "No fields to update"}), 400

        result = collection.update_one(
            {"_id": ObjectId(doc_id)},
            {"$set": update_fields},
        )
        if result.matched_count == 0:
            return jsonify({"error": "Reading not found"}), 404

        updated = collection.find_one({"_id": ObjectId(doc_id)})
        publish("reading_updated", {"reading": serialize(updated)})
        return jsonify({"success": True, "updated": update_fields})
    except Exception as exc:
        return jsonify({"error": str(exc)}), 400


@api_bp.delete("/delete/<doc_id>")
def delete_reading(doc_id):
    try:
        result = collection.delete_one({"_id": ObjectId(doc_id)})
        if result.deleted_count == 0:
            return jsonify({"error": "Reading not found"}), 404

        publish("reading_deleted", {"id": doc_id, "count": _get_count()})
        return jsonify({"success": True})
    except Exception as exc:
        return jsonify({"error": str(exc)}), 400
