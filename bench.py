import json

if __name__ == "__main__":
    with open("./bench/canada.json") as json_file:
        lj = json.loads(json_file.read())
        print(json.dumps(lj))
