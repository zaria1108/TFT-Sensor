
from os import  environ
from os.path import  abspath, dirname 
from dotenv import load_dotenv

load_dotenv()  # load environment variables from .env if it exists.
basedir = abspath(dirname(__file__))

def _as_bool(value, default=False):
    if value is None:
        return default
    return str(value).strip().lower() in ('1', 'true', 'yes', 'on')

class Config(object):
    """Base Config Object"""
    

    FLASK_DEBUG                             = _as_bool(environ.get('DEBUG'), default=False)
    SECRET_KEY                              = environ.get('SECRET_KEY', 'Som3$ec5etK*y')
    UPLOADS_FOLDER                          = environ.get('UPLOADS_FOLDER') 
    IMAGE_FOLDER                            = environ.get('IMAGE_FOLDER') 

    ENV                                     = environ.get('FLASK_ENV') 
    FLASK_RUN_PORT                          = environ.get('FLASK_RUN_PORT') 
    FLASK_RUN_HOST                          = environ.get('FLASK_RUN_HOST') 

    # MONGODB VARIABLES
    DB_USERNAME                             = environ.get('DB_USERNAME') 
    DB_PASSWORD                             = environ.get('DB_PASSWORD') 
    DB_SERVER                               = environ.get('DB_SERVER', '127.0.0.1') 
    DB_PORT                                 = environ.get('DB_PORT', '27017') 
    DB_NAME                                 = environ.get('DB_NAME', 'weather_station')
    DB_COLLECTION                           = environ.get('DB_COLLECTION', 'readings')
    MONGO_URI                               = environ.get('MONGO_URI') or environ.get('MONGODB_URI')
    MONGO_TLS                               = _as_bool(environ.get('MONGO_TLS'), default=False)

    PROPAGATE_EXCEPTIONS                    = False
 
 
