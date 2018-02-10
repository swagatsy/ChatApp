import ldap
username = "name12"
l = ldap.initialize('ldap://cs252lab.cse.iitb.ac.in:389')
username = "cn=%s,dc=cs252lab,dc=cse,dc=iitb,dc=ac,dc=in" % username 
password = "password12"
try:
	l.protocol_version = ldap.VERSION3
	l.simple_bind_s(username, password)
	valid = True
	print l
except Exception as error:
	print(error)
