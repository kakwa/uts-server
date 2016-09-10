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

        m = re.search('^#[\s](.*)', line)
        if m:
            text = m.group(1)
            if text_buf:
                text_buf += '\n'
            text_buf += text

        m = re.search('^#?([^\s]*)\s*=\s*(.*)', line)
        if m:
            var = m.group(1)
            ex_val = m.group(2)
            docs[cur_section]['vars'][var] = {'desc': text_buf, 'val': ex_val}
            text_buf = ""

max_var = 0
max_desc = 0
max_val = 0
max_section = 0

for section in docs:
    max_section = max(len(docs[section]['text']), max_section)
    for var in docs[section]['vars']:
        if docs[section]['vars'][var]:
            max_var = max(len(var), max_var)
            for line in docs[section]['vars'][var]['desc'].split('\n'):
                max_desc = max(len(line), max_desc)
            max_val = max(len(docs[section]['vars'][var]['val']), max_val)
                
def print_line(var, desc, val):
    print(\
         '| ' + var + ' ' * (max_var - len(var)) + \
        ' | ' + desc + ' ' * (max_desc - len(desc)) + \
        ' | ' + val + ' ' * (max_val - len(val)) + \
        ' |')


print('Configuration Parameters')
print('=' * len('Configuration Parameters'))
 

for section in sorted(docs):
    print('Section [ ' + section + ' ]')
    print('-' * len('Section [ ' + section + ' ]'))
    print('')
    print(docs[section]['text'])
    print('')
    print('+-' + '-' * max_var + '-+-' + '-' * max_desc + '-+-' + '-' * max_val + '-+')
    print_line('Parameter', 'Description', 'Example Value')
    print('+=' + '=' * max_var + '=+=' + '=' * max_desc + '=+=' + '=' * max_val + '=+')
    for var in sorted(docs[section]['vars']):
        if docs[section]['vars'][var]:
            first = True
            for line in docs[section]['vars'][var]['desc'].split('\n'):
                if first:
                    first = False
                    print_line(
                        re.sub('\*', '\\*', var),
                        re.sub('\*', '\\*', line),
                        re.sub('\*', '\\*', docs[section]['vars'][var]['val']),
                    )
                else:
                    print_line('', re.sub('\*', '\\*', line), '')
        print('+-' + '-' * max_var + '-+-' + '-' * max_desc + '-+-' + '-' * max_val + '-+')
    print('')
print('Full Configuration File')
print('=' * len('Full Configuration File'))
print('')
print('.. literalinclude:: ../conf/uts-server.cnf')
print('    :language: ini')
 
