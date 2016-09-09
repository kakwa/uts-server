#!/usr/bin/env python

import re
import os

import pprint

tsq_path = os.path.join(os.path.dirname(__file__),'../conf/uts-server.cnf')

docs = {}

text_buf = ""
cur_section = ""

with open(tsq_path) as f:
    for line in f.readlines():
        m = re.search('\[ (\w+) \]', line)
        if m:
            section = m.group(1)
            docs[section] = {'text': text_buf, 'vars': {}}
            cur_section = section
            text_buf = ""

        m = re.search('^#[\t ](.*)', line)
        if m:
            text = m.group(1)
            if text_buf:
                text_buf += '\n  '
            text_buf += text

        m = re.search('^#?([^\s]*)\s*=.*', line)
        if m:
            var = m.group(1)
            docs[cur_section]['vars'][var] = text_buf
            text_buf = ""

#print docs
for section in docs:
    print('### ' + section)
    print('')
    print(docs[section]['text'])
    print('')
    for var in docs[section]['vars']:
        if docs[section]['vars'][var]:
            print('* ```' + var + '```: ' + docs[section]['vars'][var])
    print('')
