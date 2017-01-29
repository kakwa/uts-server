#!/usr/bin/env python

import httplib
import time
import os
import ssl

tsq_path = os.path.join(os.path.dirname(__file__),'example.tsq')

with open(tsq_path) as f:
    body = f.read()


counter=0
old_time = time.time()
while True:
    h1 = httplib.HTTPConnection('localhost:2020')
    #h1 = httplib.HTTPSConnection('localhost:2020', context=ssl._create_unverified_context())
    h1.request('POST', '/', body, {"Content-Type": "application/timestamp-query"}) 
    response = h1.getresponse()
    h1.close()
    counter += 1
    if counter % 1000 == 0:
        new_time = time.time()
        print( str(int(1000 / (new_time - old_time))) + " req/s (1000 requests in " + str(new_time - old_time) + " seconds)")
        old_time = time.time()
