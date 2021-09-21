import mysql.connector

def connect():
    '''Connect to MySQL server'''
    return mysql.connector.connect(
        host="localhost",
        user="",     # TODO: set username for MySQL database user
        password="", # TODO: set password for MySQL database user
        database="forum",
        auth_plugin="mysql_native_password")

def checkCredentials(username, password):
    '''Check the provided credentials and return the username or false'''
    result = False
    con, cursor = None, None
    try:
        con = connect()
        # Statt den username und password in den query-String zu kopieren, wird ein Format-String
        # verwendet, um die Nutzereingaben als Parameter zu übergeben.
        # Dies ist die von MySQL empfohlene Variante.
        query = "SELECT username FROM users WHERE username = %s AND password = %s"
        cursor = con.cursor()
        cursor.execute(query, username, password)
        rows = cursor.fetchall()
        if rows:
            result = rows[0][0]
    except mysql.connector.Error as e:
        print(e)
    finally:
        if cursor: cursor.close()
        if con: con.close()
    return result

def isAdmin(username):
    '''Check if provided username is admin'''
    result = False
    con, cursor = None, None
    try:
        con = connect()
        query = "SELECT username FROM users WHERE username = %s AND admin = 1"
        cursor = con.cursor()
        cursor.execute(query, username)
        rows = cursor.fetchall()
        if rows:
            result = True
    except mysql.connector.Error as e:
        print(e)
    finally:
        if cursor: cursor.close()
        if con: con.close()
    return result

def getUsers():
    '''Get a list with all users'''
    result = []
    con, cursor = None, None
    try:
        con = connect()
        query = "SELECT * FROM users"
        cursor = con.cursor()
        cursor.execute(query)
        rows = cursor.fetchall()
        if rows:
            for row in rows:
                username = row[1]
                password = row[2]
                role = "Admin" if int(row[3]) == 1 else "User"
                mail = row[4]
                result.append([username, password, role, mail])
    except mysql.connector.Error as e:
        print(e)
    finally:
        if cursor: cursor.close()
        if con: con.close()
    return result

def getEmail(username):
    '''Return the email address for the given username or false'''
    result = False
    con, cursor = None, None
    try:
        con = connect()
        query = "SELECT mail FROM users WHERE username = %s"
        cursor = con.cursor()
        cursor.execute(query, username)
        rows = cursor.fetchall()
        if rows:
            result = rows[0][0]
    except mysql.connector.Error as e:
        print(e)
    finally:
        if cursor: cursor.close()
        if con: con.close()
    return result
