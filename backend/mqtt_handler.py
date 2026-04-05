import json
import os
import threading
import time
import uuid

import paho.mqtt.client as mqtt_client

from config import MQTT_BROKER, MQTT_PORT, MQTT_TOPIC
from database import build_document, collection, serialize
from realtime import publish

status = {
    "connected": False,
    "last_message_at": None,
    "messages_total": 0,
    "last_error": None,
}

_RC_MEANINGS = {
    1: "wrong protocol version",
    2: "client ID rejected",
    3: "broker unavailable",
    4: "bad credentials",
    5: "not authorised",
}


def _rc_message(rc) -> str:
    if rc == 0:
        return "success"

    return _RC_MEANINGS.get(rc, mqtt_client.error_string(rc))


def _publish_status():
    publish(
        "mqtt_status",
        {
            "connected": status["connected"],
            "last_message_at": status["last_message_at"],
            "messages_total": status["messages_total"],
            "last_error": status["last_error"],
        },
    )


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        status["connected"] = True
        status["last_error"] = None
        client.subscribe(MQTT_TOPIC)
        _publish_status()
        print(f"[MQTT] Connected to broker as {client._client_id.decode('utf-8', errors='ignore')}")
        print(f"[MQTT] Subscribed -> {MQTT_TOPIC}")
    else:
        status["connected"] = False
        status["last_error"] = _rc_message(rc)
        _publish_status()
        print(f"[MQTT] Connection refused -> {status['last_error']}")


def on_disconnect(client, userdata, rc):
    status["connected"] = False
    if rc == 0:
        status["last_error"] = None
        _publish_status()
        print("[MQTT] Clean disconnect")
    else:
        status["last_error"] = f"unexpected disconnect rc={rc}: {_rc_message(rc)}"
        _publish_status()
        print(f"[MQTT] {status['last_error']} -> will reconnect")


def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8")
        data = json.loads(payload)
        document = build_document(data)
        result = collection.insert_one(document)
        status["messages_total"] += 1
        status["last_message_at"] = document["timestamp"].isoformat()
        reading = serialize({**document, "_id": result.inserted_id})
        publish("reading_created", {"reading": reading, "count": collection.count_documents({})})
        _publish_status()
        print(f"[MQTT] Saved -> {result.inserted_id}")
    except json.JSONDecodeError as exc:
        status["last_error"] = f"bad JSON: {exc}"
        _publish_status()
        print(f"[MQTT] Invalid JSON -> {exc}")
    except Exception as exc:
        status["last_error"] = str(exc)
        _publish_status()
        print(f"[MQTT] DB error -> {exc}")


def _run():
    delay = 5
    client_id = f"flask_weather_sub_{os.getpid()}_{uuid.uuid4().hex[:8]}"
    client = mqtt_client.Client(client_id=client_id, clean_session=True)
    client.on_connect = on_connect
    client.on_disconnect = on_disconnect
    client.on_message = on_message
    client.reconnect_delay_set(min_delay=1, max_delay=5)

    while True:
        try:
            print(f"[MQTT] Connecting to {MQTT_BROKER}:{MQTT_PORT} ...")
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            client.loop_forever()
        except OSError as exc:
            status["connected"] = False
            status["last_error"] = str(exc)
            _publish_status()
            print(f"[MQTT] Network error -> {exc} -> retry in {delay}s")
            time.sleep(delay)
            delay = min(delay * 2, 60)
        except Exception as exc:
            status["last_error"] = str(exc)
            _publish_status()
            print(f"[MQTT] Unexpected error -> {exc} -> retry in {delay}s")
            time.sleep(delay)
            delay = min(delay * 2, 60)
        else:
            delay = 5


def start_mqtt_thread() -> threading.Thread:
    thread = threading.Thread(target=_run, daemon=True, name="mqtt-subscriber")
    thread.start()
    print("[MQTT] Subscriber thread started")
    return thread
