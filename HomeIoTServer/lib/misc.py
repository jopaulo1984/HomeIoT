
def httpsend(host, port=80, ifile="/", content="", method="GET", contenttype="text/plain", accept="text/plain"):
    httpclient = client.HTTPConnection(host, port)
    httpclient.request(method, ifile, cmd, {"Content-type": contenttype, "Accept": accept})
    response = httpclient.getresponse().read().decode()
    httpclient.close()
    return response