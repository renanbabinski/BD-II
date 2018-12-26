arquivo = open("teste2", 'r')
transactions = []
variables = []

n = int(input("Digite o numero de variaveis do log:"))

for i in range(ord("A"),ord("A")+n):
    print("Digite o valor inicial da variavel: ", chr(i))
    variables.append((chr(i), int(input())))

print("Variaveis Gravadas!", variables)
input()

for linha in arquivo.readlines():
    s = linha.split(",")

    if(s[0] == "start"):
        transactions.append((s[1], False, False))
    elif(s[0] == "Commit"):
        for i in range(len(transactions)):
            if(transactions[i][0] == s[1]):
                transactions[i] = (s[1], True, False)
                break;

print("Transações Comitadas e não commitadas", transactions)
input()

print("###########O BANCO QUEBROU!!!!!#############")
print("###########REDO LOG EM PROCESSO DE RECUPERAÇÂO...#############")
input()

arquivo.seek(0)

for linha in arquivo.readlines():
    s = linha.split(",")

    if(s[0] == "write"):
        for i in range(len(transactions)):
            if(transactions[i][0] == s[1]):
                if(transactions[i][1] == False):
                    break;
                else:
                    for i in range(len(variables)):
                        if(variables[i][0] == s[2]):
                            variables[i] = (s[2], s[3])
                            break;

print("Valores finais das variaveis:")
print(variables)

arquivo.close()












