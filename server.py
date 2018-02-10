import re
import csv
import socket 
from threading import Thread 
from SocketServer import ThreadingMixIn 


# maintain mapping of active users to their socket connection
dict = {}

# Multithreaded Python server : TCP Server Socket Thread Pool
class ClientThread(Thread): 
 
    def __init__(self,ip,port,conn): 
        Thread.__init__(self) 
        self.ip = ip              # IP Address of Client
        self.port = port          # Port number of Client
        self.conn = conn          # Socket Connection of Server-Client for sending and recieving data
        print ("[+] New server socket thread started for " + ip + ":" + str(port) )
 
    def run(self): 
        glob_temp = 0
        glob_username = ''
        while True : 
            data = self.conn.recv(2048)    # Recieving data from client
            print ("data from " + str(self.port) +":" + data)

            # Parsing of data from client to list of strings
            a = re.split(';|,|:',data)    

            # Registration of user
            if a[0]== "r":
                name = a[1]
                username = a[2]
                password = a[3]
                temp =0
                # update database with the new registered user
                with open('database.csv','r') as f:
                    file = csv.reader(f,delimiter = ',')
                    for row in file:
                        if(row[1] == username):
                                temp = 1
                                message = "\nThis username already exists. Choose another username\n"
                f.close()
                if(temp == 0):
                    with open('database.csv','a') as f:
                        f.write(name + ","+username+","+password+"\n")
                    f.close()
                    message = "registered\n" 
                self.conn.send(message)     

            # Login User
            elif a[0]== "l":
                username = a[1]
                password = a[2]


                # l = ldap.initialize('ldap://cs252lab.cse.iitb.ac.in:389')
                # username = "cn=%s,dc=cs252lab,dc=cse,dc=iitb,dc=ac,dc=in" % username 
                # try:
                #     l.protocol_version = ldap.VERSION3
                #     l.simple_bind_s(username, password)
                #     valid = True
                #     glob_username = username
                #     dict[username] = self.conn
                #     message = "Authenticated\n"
                # except Exception as error:
                #     message = "Bad username or password"+"\n"


                # Checking in database if user with given credentials exist
                with open('database.csv','r') as f:
                    file = csv.reader(f ,delimiter=',')
                    temp=0;
                    for row in file:
                        if (row[1] == username and row[2]==password):
                            message  = "Authenticated\n"
                            glob_username = username
                            temp = 1
                            dict[username] = self.conn
                            break
                    # if details of user are not valid
                    if (temp == 0):
                        message = "Bad username or password"+"\n"
                f.close()
                self.conn.send(message)

            # List of messages that are sent to user when he/she was not logged in(Unseen Messages)
            elif a[0] == "u":
                message_buffer = ""
                with open('message_unseen.csv', 'rb') as inp, open('message_unseen_edit.csv', 'wb') as out,open('message_seen.csv', 'a') as ff:
                    writer = csv.writer(out)
                    for row in csv.reader(inp):
                        if row[1] != glob_username:
                            writer.writerow(row)
                        else:
                            message_buffer = message_buffer + row[0]+" : " + row[2]+"\n"
                            ff.write(row[0]+","+row[1]+","+row[2]+"\n")
                inp.close()
                out.close()
                ff.close()
                with open('message_unseen_edit.csv', 'rb') as inp, open('message_unseen.csv', 'wb') as out:
                    writer = csv.writer(out)
                    for row in csv.reader(inp):
                        writer.writerow(row)
                inp.close()
                out.close()
                if( message_buffer != ""):    
                    message_buffer = "Unseen messages :-\n" + message_buffer
                    self.conn.send(message_buffer)
                else:
                    message = "No unseen messages"
                    self.conn.send(message) 

            # this option provides a user to send same message to a group of people.
            elif a[0] == "m":
                num = int(a[1])
                msg = a[len(a)-1]
                temp =0
                for entry in dict:
                    if (dict[entry] == self.conn):
                        username = entry
                        temp = 1
                        break
                if (temp==0):
                    message  = "first get logged in"
                else:
                    with open('message_unseen.csv','a') as f:
                        for i in range(2,len(a)-1):
                            for entry in dict:
                                if ( entry != a[i] and entry != username):
                                    f.write(username+","+a[i]+","+msg+"\n")
                    f.close()
                    with open('message_seen.csv','a') as f:
                        for i in range(2,len(a)-1):
                            for entry in dict:
                                if ( entry == a[i]):
                                    message_to_send = username + ": " +msg
                                    dict[entry].send(message_to_send)
                                    f.write(username+","+a[i]+","+msg+"\n")
                    f.close()
             
            # return  list of friends online 
            elif a[0]=="o":     
                message = "Freinds online:-\n"
                for entry in dict:
                    if ( entry != glob_username):
                        message = message + "\t" + entry + "\n"
                self.conn.send(message)

            # return registered username
            elif a[0]=="f":      
                with open( 'database.csv','r') as f:
                    file = csv.reader(f ,delimiter=',')
                    message = "Freinds Registered:-\n"
                    for row in file:
                        if (row[1] != glob_username):
                            message  =  message + "\t" + row[1] + "\n"
                f.close()
                self.conn.send(message)

            # return prevous history with freind
            elif a[0] == "h":
                friend = a[1]
                message = ""
                with open( 'message_seen.csv','r') as f:
                    file = csv.reader(f ,delimiter=',')
                    for row in file:
                        if( friend == row[1] and row[0]==glob_username):
                            message = message + "Sent: " + row[2] +"\n"
                        elif ( row[1]== glob_username  and row[0]==friend):
                            message = message + "Recieved: " + row[2] + "\n"
                        else:
                            message = message
                f.close()
                with open( 'message_unseen.csv','r') as f:
                    file = csv.reader(f ,delimiter=',')
                    for row in file:
                        if( friend == row[1] and row[0]==glob_username):
                            message = message + "Sent: " + row[2] +"\n"
                        elif ( row[1]== glob_username  and row[0]==friend):
                            message = message + "Recieved: " + row[2] + "\n"
                        else:
                            message = message
                f.close()
                self.conn.send(message)

            # Peer-to-peer connection
            elif a[0] == "p":
                friend = a[1]
                message = glob_username + "wants to chat with you peer-to-peer\n"
                dict[friend].send(message)
                self.conn.send("Request has been successfully sent")

            # Log out
            elif a[0]=="e":
                temp = 0
                for entry in dict:
                    if (dict[entry] == self.conn):
                        username = entry
                        glob_username = username
                        del dict[username]
                        message = "logged out"
                        temp = 1
                        glob_temp  = 1
                        self.conn.send(message)
                        break
                if ( temp == 0):
                    message = "you have not logged in yet"
                    self.conn.send(message)

            # if some unknown command is given by user
            else:
                message  = "bad Query"
                self.conn.send(message)

 
            if( glob_temp ==1) :
                print (glob_username + " has logged out")
                break

# Multithreaded Python server : TCP Server Socket Program Stub
TCP_IP = '0.0.0.0' 
TCP_PORT = 4004
 
tcpServer = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
tcpServer.bind((TCP_IP, TCP_PORT)) 
threads = []

maxcount  = 10;
tcpServer.listen(maxcount) 
#print "Multithreaded Python server : Waiting for connections from TCP clients..." 
while True:
    (conn, (ip,port)) = tcpServer.accept()
    print (str(port) + "connected")
    newthread = ClientThread(ip,port,conn)
    newthread.start()
    threads.append(newthread)

for t in threads:
    t.join()