import requests
from hello_pb2 import Point

point = Point()
point.x = 7
point.y = 8
point.z = 9
print(point)
response = requests.post(
    "http://127.0.0.1:8090/cyclone/examples/simple-pb/", point.SerializeToString())
print(response)
