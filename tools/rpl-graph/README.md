This tool produces a graph of the RPL nodes DoDAG, as available
via http requests to rpl-border-router and http-server nodes.

You need:
- rpl-border-router (with serial port on your PC)
- tunslip6 (prefix aabb) bridging the BR to your PC
- http-server nodes (examples/ipv6/http-server)
- graphviz (to generate .png or .svg files)
- python, with package pydot installed
- the script

Output:
- RPL.dot (a graph) with information collected as per http requests
- create .png (or .svg) using graphviz's ``dot -oRPL.png -Tpng RPL.dot``

Notes:
- prefix ``aabb`` is hardcoded
- border router address is hardcoded
- won't work on recent http-server code (redo parsing)
- blue links show RPL network routes
- grey links show radio conenctivity (neighbours)
- red links are currently 'useless'
- maybe replace ``if Retries[sourceaddr]==4`` with ``==6``
