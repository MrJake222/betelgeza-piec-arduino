import json

params = json.load(open("parametry_pieca.json")) 
desc = open("parametry_lista").read()

out = open("parametry_tabela.txt", "w")
out2 = open("parametry_tabela.csv", "w")
D = {}

def write(*args):
	out.write(" ".join(args) + "\n")
	out2.write(",".join(args) + "\n")
	print(*args)

for key in params.keys():
	val = str(params[key])
	while len(key) < 4: key = "0" + key
	while len(val) < 8: val = " " + val
	try:
		i = desc.index("#" + key.upper())
		i = desc.index("<td>", i)
		i = desc.index("<td>", i+1)
		j = desc.index("</td>", i)
		write(key, val, desc[i+4:j])
		D[key] = desc[i+4:j]
		# break
	except ValueError:
		# continue
		write(key, val, "---")

out.close()
out2.close()

json.dump(D, open("parametry_tabela.json", "w"))
