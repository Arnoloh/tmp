import requests
import time
import subprocess as sp
import signal
import socket

HOST:str = "127.0.0.2"
PORT:str = "1312"


EXE:str = "./httpd"

def spawn_httpd(httpd_exec: str, config_file: str, stdout_filename: str)-> sp.Popen:
    with open(stdout_filename,"w") as f:
        httpd_process = sp.Popen(
            [httpd_exec,config_file],stdout=f, stderr=sp.PIPE,bufsize=0
        )
        time.sleep(0.1)
        return httpd_process


def kill_httpd(process: sp.Popen):
    process.send_signal(signal.SIGINT)


def start_server():
    proc = spawn_httpd(EXE,"tests/server.conf","out.log")
    return proc


def test_correct_with_space():
    proc = start_server()
    try:
        response = requests.get(f"http://{HOST}:{PORT}/inde%20x.html",timeout = 1)
    except requests.ConnectionError:
        raise ConnectionError()
        kill_httpd(proc)
        assert False
        return
    assert response.status_code == 200
    with open("public/inde x.html","r") as f:
        assert response.text == f.read()

    kill_httpd(proc)





def test_get_index():
    proc = start_server()
    try:
        response = requests.get(f"http://{HOST}:{PORT}/",timeout = 1)
    except requests.ConnectionError:
        raise ConnectionError()
        kill_httpd(proc)
        assert False
        return
    assert response.status_code == 200
    with open("public/index.html","r") as f:
        assert response.text == f.read()
        kill_httpd(proc)

def test_get_index_sub_dir():
    proc = start_server()
    try:
        response = requests.get(f"http://{HOST}:{PORT}/test/index.html",timeout = 1)
    except requests.ConnectionError:
        raise ConnectionError()
        kill_httpd(proc)
        assert False
        return
    assert response.status_code == 200
    with open("public/test/index.html","r") as f:
        assert response.text == f.read()
    kill_httpd(proc)


def test_get_invalid():
    proc = start_server()
    try:
        response = requests.get(f"http://{HOST}:{PORT}/test/tml",timeout = 1)
    except requests.ConnectionError:
        raise ConnectionError()
        kill_httpd(proc)
        assert False
        return
    assert response.status_code == 404
    kill_httpd(proc)

def test_wrong_method():
    proc = start_server()
    requests = "PUT /index.html HTTP/1.1\r\nHost: 127.0.0.2:1312\r\n\r\n"
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    sock.sendall(requests.encode())
    response = sock.recv(1024)
    resp = response.decode()
    assert "405 Method Not Allowed" in resp
    kill_httpd(proc)


def test_BR():
    proc = start_server()
    requests = "GET /index.html FTP/1.1\r\nHost: 127.0.0.2:1312\r\n\r\n"
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    sock.sendall(requests.encode())
    response = sock.recv(1024)
    resp = response.decode()
    assert "400 Bad Request" in resp
    kill_httpd(proc)



def test_MNA():
    proc = start_server()
    requests = "GET /index.html HTTP/1.0\r\nHost: 127.0.0.2:1312\r\n\r\n"
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    sock.sendall(requests.encode())
    response = sock.recv(1024)
    resp = response.decode()
    assert "505 HTTP Version Not Supported" in resp
    kill_httpd(proc)




def test_Null_Byte():
    proc = start_server()
    requests = "GET /ind\0ex.html HTTP/1.1\r\nHost: 127.0.0.2:1312\r\n\r\n"
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    sock.sendall(requests.encode())
    response = sock.recv(1024)
    resp = response.decode()
    assert "404 Not Found" in resp
    kill_httpd(proc)



def test_correct_with_body():
    proc = start_server()
    requests = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.2:1312\r\nContent-Length: 8\r\n\r\n"
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    sock.sendall(requests.encode())
    requests = "Hello World je suis un useless body"
    sock.sendall(requests.encode())
    response = sock.recv(1024)
    resp = response.decode()
    assert "200 OK" in resp
    kill_httpd(proc)

def test_correct_with_body_send_2_times_all_2():
    proc = start_server()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    requests = "GET /index.html HTTP/1.1\r\n"
    sock.sendall(requests.encode())
    requests = "Host: 127.0.0.2:1312\r\nContent-Length: 8\r\n\r\n"
    sock.sendall(requests.encode())
    resp = b""
    try:
        resp = sock.recv(1,socket.MSG_DONTWAIT)
    except BlockingIOError as e:
        pass
    if resp != b"":
        kill_httpd(proc)
        assert False
    requests = b"Hello"
    sock.sendall(requests)
    resp = b""
    try:
        resp = sock.recv(1,socket.MSG_DONTWAIT)
    except BlockingIOError as e:
        pass
    if resp != b"":
        kill_httpd(proc)
        assert False

    requests = b"hde"
    sock.sendall(requests)

    response = sock.recv(1024)
    resp = response.decode()
    assert "200 OK" in resp
    kill_httpd(proc)



def test_correct_with_body_send_2_times_all_close():
    proc = start_server()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    requests = "GET /index.html HTTP/1.1\r\n"
    sock.sendall(requests.encode())
    requests = "Host: 127.0.0.2:1312\r\nConnection: close\r\nContent-length: 8\r\n\r\n"
    sock.sendall(requests.encode())
    resp = b""
    try:
        resp = sock.recv(1,socket.MSG_DONTWAIT)
    except BlockingIOError as e:
        pass
    if resp != b"":
        kill_httpd(proc)
        assert False
    requests = b"Hello"
    sock.sendall(requests)
    resp = b""
    try:
        resp = sock.recv(1,socket.MSG_DONTWAIT)
    except BlockingIOError as e:
        pass
    if resp != b"":
        kill_httpd(proc)
        assert False

    requests = b"hde"
    sock.sendall(requests)

    response = sock.recv(1024)
    resp = response.decode()
    assert "200 OK" in resp
    kill_httpd(proc)


def test_correct_with_body_send_0x0():
    proc = start_server()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    requests = "GET /index.html HTTP/1.1\r\n"
    sock.sendall(requests.encode())
    requests = b"Host: 127.0.0.2:1312\r\nContent-Length: 8\r\n\r\n123\0a678"
    sock.sendall(requests)
    response = sock.recv(1024)
    resp = response.decode()
    assert "200 OK" in resp
    kill_httpd(proc)

def send_socket(requests, code):
    proc = start_server()
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, int(PORT)))
    sock.sendall(requests.encode())
    response = sock.recv(1024)
    resp = response.decode()
    assert code in resp

    kill_httpd(proc)


def test_wrong_version():
    requests = "GET /index.html HTTP/1.0\r\nHost: 127.0.0.2:1312\r\n\r\n"
    code = "505"
    send_socket(requests, code)
    
def test_bad_http_version_missing_digit():
    requests = "GET /index.html HTTP/1.\r\nHost: 127.0.0.2:1312\r\n\r\n"
    code = "400"
    send_socket(requests, code)


def test_double_host():
    requests = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.2:1312\r\nHost: 127.0.0.2\r\n\r\n"
    code = "400"
    send_socket(requests, code)

def test_negative_length():
    requests = "GET /index.html HTTP/1.1\r\nHost: 127.0.0.2\r\nContent-Length: -4\r\n\r\ndjaskdja"
    code = "400"
    send_socket(requests, code)

def test_FULL_uri():
    requests = "HEAD http://Arnoloh/index.html HTTP/1.1\r\nHost: 127.0.0.2\r\n\r\ndjaskdja"
    code = "200"
    send_socket(requests, code)

def test_FULL_case_sensitive1():
    requests = "HEAD http://Arnoloh/index.html HTTP/1.1\r\nHOST: 127.0.0.2\r\n\r\ndjaskdja"
    code = "200"
    send_socket(requests, code)

def test_FULL_case_sensitive2():
    requests = "HEAD http://Arnoloh/index.html HTTP/1.1\r\nHOST: 127.0.0.2\r\ncontent-length: 8\r\n\r\ndjaskdjasdfgdsaf"
    code = "200"
    send_socket(requests, code)

    
