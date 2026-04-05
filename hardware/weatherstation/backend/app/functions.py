 #!/usr/bin/python3


#################################################################################################################################################
#                                                    CLASSES CONTAINING ALL THE APP FUNCTIONS                                                                                                    #
#################################################################################################################################################


class DB:

    def __init__(self,Config):

        from math import floor
        from os import getcwd
        from os.path import join
        from json import loads, dumps, dump
        from datetime import timedelta, datetime, timezone 
        from pymongo import MongoClient , errors, ReturnDocument
        from urllib import parse
        from urllib.request import  urlopen 
        from bson.objectid import ObjectId  
       
      
        self.Config                         = Config
        self.getcwd                         = getcwd
        self.join                           = join 
        self.floor                      	= floor 
        self.loads                      	= loads
        self.dumps                      	= dumps
        self.dump                       	= dump  
        self.datetime                       = datetime
        self.ObjectId                       = ObjectId 
        self.server			                = Config.DB_SERVER
        self.port			                = Config.DB_PORT
        self.username                   	= parse.quote_plus(Config.DB_USERNAME or "")
        self.password                   	= parse.quote_plus(Config.DB_PASSWORD or "")
        self.db_name                        = Config.DB_NAME
        self.collection_name                = Config.DB_COLLECTION
        self.mongo_uri                      = Config.MONGO_URI
        self.remoteMongo                	= MongoClient
        self.ReturnDocument                 = ReturnDocument
        self.PyMongoError               	= errors.PyMongoError
        self.BulkWriteError             	= errors.BulkWriteError  
        self.tls                            = Config.MONGO_TLS
        self._client                        = None
        self._collection                    = None
        self._connect()


    def __del__(self):
            # Delete class instance to free resources
            try:
                if self._client is not None:
                    self._client.close()
            except Exception:
                pass


    def _build_mongo_uri(self):
        if self.mongo_uri:
            return self.mongo_uri

        if self.username and self.password:
            return f"mongodb://{self.username}:{self.password}@{self.server}:{self.port}"

        return f"mongodb://{self.server}:{self.port}"


    def _connect(self):
        uri = self._build_mongo_uri()
        self._client = self.remoteMongo(uri, tls=self.tls, serverSelectionTimeoutMS=5000)
        self._client.admin.command("ping")
        self._collection = self._client[self.db_name][self.collection_name]
        return self._collection


    def _get_collection(self):
        if self._collection is None:
            return self._connect()
        return self._collection
 


    ####################
    # LAB 2 DATABASE UTIL FUNCTIONS  #
    ####################
    
    def addUpdate(self,data):
        '''ADD A NEW STORAGE LOCATION TO COLLECTION'''
        try:
            result = self._get_collection().insert_one(data)
        except Exception as e:
            msg = str(e)
            if "duplicate" not in msg:
                print("addUpdate error ",msg)
            return False
        else:                  
            return True
        
       

    def getAllInRange(self,start, end):
        '''RETURNS A LIST OF OBJECTS. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            result = list(self._get_collection().find('''Add your query here'''))
        except Exception as e:
            msg = str(e)
            print("getAllInRange error ",msg)            
        else:                  
            return result
        

    def humidityMMAR(self,start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR HUMIDITY. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            result = list(self._get_collection().aggregate('''Add your Aggregation pipeline here in this function'''))
        except Exception as e:
            msg = str(e)
            print("humidityMMAS error ",msg)            
        else:                  
            return result
        
    def temperatureMMAR(self,start, end):
        '''RETURNS MIN, MAX, AVG AND RANGE FOR TEMPERATURE. THAT FALLS WITHIN THE START AND END DATE RANGE'''
        try:
            result = list(self._get_collection().aggregate('''Add your Aggregation pipeline here in this function'''))
        except Exception as e:
            msg = str(e)
            print("temperatureMMAS error ",msg)            
        else:                  
            return result


    def frequencyDistro(self,variable,start, end):
        '''RETURNS THE FREQUENCY DISTROBUTION FOR A SPECIFIED VARIABLE WITHIN THE START AND END DATE RANGE'''
        try:
            result = list(self._get_collection().aggregate('''Add your Aggregation pipeline here in this function'''))
        except Exception as e:
            msg = str(e)
            print("frequencyDistro error ",msg)            
        else:                  
            return result
        
 



def main():
    from config import Config
    from time import time, ctime, sleep
    from math import floor
    from datetime import datetime, timedelta
    one = DB(Config)
 
 
    start = time() 
    end = time()
    print(f"completed in: {end - start} seconds")
    
if __name__ == '__main__':
    main()


    
