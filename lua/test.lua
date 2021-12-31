gg.searchNumber("19D;72D::16", gg.TYPE_AUTO)
local c=gg.getResultsCount()
local r=gg.getResults(2)
r=gg.getValues(r)
print('First 5 results: ', r)
print('First result: ', r[1])
print('First result address: ', r[1].address)
print('First result value: ', r[1].value)
print('First result type: ', r[1].flags)
local ex = {}
ex[1].address = r[1].address + 8
ex[1].flags = r[1].flags
print('local move ', ex[1].value)