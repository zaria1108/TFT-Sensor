import json
from queue import Full, Queue
from threading import Lock

_subscribers = set()
_lock = Lock()


def subscribe(maxsize: int = 100) -> Queue:
    queue = Queue(maxsize=maxsize)
    with _lock:
        _subscribers.add(queue)
    return queue


def unsubscribe(queue: Queue) -> None:
    with _lock:
        _subscribers.discard(queue)


def publish(event: str, data: dict) -> None:
    with _lock:
        subscribers = list(_subscribers)

    stale = []
    for queue in subscribers:
        try:
            queue.put_nowait({"event": event, "data": data})
        except Full:
            try:
                queue.get_nowait()
                queue.put_nowait({"event": event, "data": data})
            except Exception:
                stale.append(queue)
        except Exception:
            stale.append(queue)

    if stale:
        with _lock:
            for queue in stale:
                _subscribers.discard(queue)


def encode_sse(event: str, data: dict) -> str:
    return f"event: {event}\ndata: {json.dumps(data)}\n\n"
