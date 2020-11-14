f = open("file_tx.txt", "w")
for i in range(0, 10000):
    f.write("This is line number %d\r\n" % i)
f.close()