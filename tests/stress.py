#!/usr/bin/env python

import httplib
import time
import os

tsq_path = os.path.join(os.path.dirname(__file__),'example.tsq')

with open(tsq_path) as f:
    body = f.read()


counter=0
old_time = time.time()
while True:
    h1 = httplib.HTTPConnection('localhost:2020')
    h1.request('POST', '/', body, {"Content-Type": "application/timestamp-query"}) 
    response = h1.getresponse()
    h1.close()
    counter += 1
    if counter % 10000 == 0:
        new_time = time.time()
        print( str(int(10000 / (new_time - old_time))) + " req/s (10000 requests in " + str(new_time - old_time) + " seconds)")
        old_time = time.time()
