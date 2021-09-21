from bottle import get, post, run, template, static_file, redirect, response, request
import database as db

'''Stores forum messages (non-persistent)'''
messages = []

def start(port):
    '''Start webserver locally at given port'''
    run(host="localhost", port=port)

@get("/css/style.css")
def getCss():
    '''Handle GET /css/style.css'''
    return static_file("css/style.css", root="./templates/")

@get("/")
def getIndex():
    '''Handle GET /'''
    username = request.get_cookie("username")
    if username:
        isAdmin = db.isAdmin(username)
        return template("templates/index.html", username=username, messages=messages, isAdmin=isAdmin)
    else:
        return redirect("/login")

@post("/")
def postIndex():
    '''Handle POST /'''
    username = request.get_cookie("username")
    message = request.forms.get("message")
    if username and message:
        messages.append([username, message])
    redirect("/")

@get("/login")
def getLogin():
    '''Handle GET /login'''
    return template("templates/login.html")

@post("/login")
def postLogin():
    '''Handle POSTS to /login'''
    username = request.forms.get("username")
    password = request.forms.get("password")
    username = db.checkCredentials(username, password)
    if username:
        response.set_cookie("username", username)
        redirect("/")
    else:
        return template("templates/error.html")

@get("/logout")
def logout():
    '''Log the user out via GET /logout'''
    response.delete_cookie("username")
    redirect("/")

@get("/users")
def getUsers():
    '''Handle GET /users'''
    username = request.get_cookie("username")
    if username:
        if db.isAdmin(username):
            users = db.getUsers()
            return template("templates/users.html", username=username, users=users, isAdmin=True)
    else:
        redirect("/")

@get("/retrieve_password")
def getRetrievePassword():
    '''Handle GET /retrieve_password'''
    return template("templates/retrieve_password.html", send=False)

@post("/retrieve_password")
def postRetrievePassword():
    '''Handle POST /retrieve_password'''
    username = request.forms.get("username")
    email = db.getEmail(username)
    if email:
        # TODO: Imagine that email with password got sent here!
        return template("templates/retrieve_password.html", send=True)
    else:
        return template("templates/error.html")
