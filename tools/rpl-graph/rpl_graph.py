import re
import urllib2
import pydot
import socket

def processaddr(sourceaddr):
  print "processing ", sourceaddr , "(" , NodeList.index(sourceaddr)+1 , "/" , len(NodeList) , ")"
  stripedsourceaddr=re.sub(":",".", sourceaddr)
  try:
      raw=urllib2.urlopen("http://[aabb::"+sourceaddr+"]",timeout=30).read()
      VisitedList.append(sourceaddr)
      res=re.split("\n|<pre>", raw)
      readingroutes=0
      for addr in res:
          if readingroutes==0 :
              if "Routes" in addr:
                  readingroutes=1
              elif '::' in addr :
                  stripedaddr=re.sub(".*::","",addr)
                  if stripedaddr in NodeList :
                      print stripedaddr+" is also near "+sourceaddr
                  else :
                      print stripedaddr+" is new, near "+sourceaddr
                      NodeList.append(stripedaddr)
                  graph.add_node(pydot.Node(stripedsourceaddr))
                  graph.add_edge(pydot.Edge(stripedsourceaddr,re.sub(":",".",stripedaddr),color="grey"))
          elif "::" in addr :
              pair=[]
              pair=re.split("\/128 \(via |\) lus",addr)
              pair[0]=re.sub(".*::","",pair[0])
              pair[1]=re.sub(".*::","",pair[1])
              if pair[0]==pair[1] :
                  print "New direct route from "+ sourceaddr+" to "+ pair[0]
                  graph.add_edge(pydot.Edge(stripedsourceaddr,re.sub(":",".",pair[0]),color="blue"))
              else :
                  print "New route to "+ pair[0]+" via "+ pair[1]
                  graph.add_edge(pydot.Edge(re.sub(":",".",pair[1]),re.sub(":", ".",pair[0]),color="red"))

  except (urllib2.URLError,socket.timeout):
    print "node "+sourceaddr+" not responding"
    if sourceaddr in Retries :
        Retries[sourceaddr]=Retries[sourceaddr]+1
        if Retries[sourceaddr]==4 :
            print "  and will not be tried again"
    else :
        Retries[sourceaddr]=1

  for addr in NodeList :
     if not addr in VisitedList :
         if (not addr in Retries) or Retries[addr]<6 : 
             processaddr(addr)

BRaddr="323:4501:1782:343"
NodeList=[]
VisitedList=[]
Routes=[]
Retries={}
graph = pydot.Dot(graph_type='digraph')

graph.add_node(pydot.Node(re.sub(":",".",BRaddr), shape="box"))

NodeList.append(BRaddr)
processaddr(BRaddr)

#BRstriped=re.sub(".*::","",BRaddr)
 
print NodeList           
print Routes

graph.write('RPL.dot')

