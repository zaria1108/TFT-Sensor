from datetime import datetime, timezone
from pymongo import MongoClient
from config import MONGO_URI, DB_NAME, COLLECTION, JAMAICA_TZ

mongo_client = MongoClient(MONGO_URI)
db           = mongo_client[DB_NAME]
collection   = db[COLLECTION]
collection.create_index("timestamp")


def format_timestamp(ts: datetime) -> str:
    """Convert a UTC datetime to a Jamaica-local formatted string."""
    return (
        ts.replace(tzinfo=timezone.utc)
          .astimezone(JAMAICA_TZ)
          .strftime("%Y-%m-%d %H:%M:%S")
    )


def build_document(data: dict) -> dict:
    """Build a readings document from a raw payload dict."""
    return {
        "timestamp"       : datetime.utcnow(),
        "temperature"     : data.get("temperature"),
        "humidity"        : data.get("humidity"),
        "bmp_temperature" : data.get("bmp_temperature"),
        "pressure"        : data.get("pressure"),
        "soil_moisture"   : data.get("soil_moisture"),
    }


def serialize(record: dict) -> dict:
    """Convert a MongoDB document to a JSON-safe dict."""
    record["id"] = str(record.pop("_id"))
    record["timestamp"] = format_timestamp(record["timestamp"])
    return record