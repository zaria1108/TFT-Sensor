from flask import Flask, render_template

from config       import FLASK_PORT
from mqtt_handler import start_mqtt_thread
from routes       import api_bp

# ── App factory ───────────────────────────────────────────────────────────────

def create_app() -> Flask:
    app = Flask(__name__, template_folder="templates")
    app.register_blueprint(api_bp)

    @app.get("/")
    def dashboard():
        return render_template("dashboard.html")

    return app


# ── Entrypoint ────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    start_mqtt_thread()
    app = create_app()
    print(f"[Flask] Dashboard → http://localhost:{FLASK_PORT}")
    app.run(host="0.0.0.0", port=FLASK_PORT, debug=False)