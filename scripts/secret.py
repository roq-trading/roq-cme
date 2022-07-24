#!/usr/bin/env python

from base64 import b64encode

from hashlib import sha256

key = '2tZQEQRV'
secret = 'saQaP6WmDefitTmd6DcAqnhJFtpC9eubZ3bzYm21af4'

timestamp = 1567069890531
nonce = 'LM2L/w9Q3ZVbRxNDlx7FfrqSrq+eeMlb6wAtRocgFLY='

raw_data = '{}.{}'.format(timestamp, nonce)

print(raw_data)

m = sha256()
m.update(raw_data.encode('utf-8'))
m.update(secret.encode('utf-8'))

d = m.digest()

b = b64encode(d).decode('utf-8')

print(b)
print(len(b))



m = sha256()
m.update((raw_data + secret).encode('utf-8'))

d = m.digest()

b = b64encode(d).decode('utf-8')

print(b)
