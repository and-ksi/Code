GG修改器lua脚本常用函数总结:
所有函数参阅官网网站
https://gameguardian.net/help/classgg.html
1.弹一个提示框
gg.alert("输入不能为空!")
local ex = gg.alert('你是否要退出脚本?', '是', nil, '否')
if ex == 1 then    --返回1为"是"  3为"否"
    os.exit()    --退出脚本
end
                 os.exit()--退出脚本 end
2.弹出一个列表菜单
local t = gg.choice({"第一项","第二项"})--返回选择项的索引，从1开始
if t == 1 then
    gg.alert("第一项!")
end
3.弹出一个具有编辑互交功能的列表菜单
--gg.prompt(t1, t2, t3)  t1代表列表项的菜单名称，t2代表互交控件的默认值，t3代表需要展示什么控件， 可参阅types: 'number', 'text', 'path', 'file', 'new_file', 'setting', 'speed', 'checkbox'，返回值是控件里面值的列表
local t = gg.prompt(
{"姓名", "年龄"},
{[1]="张三", [2]=20},--如果没有默认值写  nil
{[1]="text",[2]="number"})--其中 "[数字]= "可省略
if t == nil then
    gg.alert("你选择了取消")
else
    gg.alert("你的姓名:"..t[1].."年龄:"..t[2])
end
4.复选框菜单
local t = gg.multiChoice({'A', 'B', 'C', 'D'})
if t == nil then
    gg.alert('Canceled')
else
    if t[1] then
        gg.alert('do A')
    end
    if t[2] then
        gg.alert('do B')
    end
    if t[3] then
        gg.alert('do C')
    end
    if t[4] then
        gg.alert('do D')
    end
end
5.清除搜索列表
gg.clearResults()
6.搜索一个数值[最常用]
gg.searchNumber("100", gg.TYPE_DWORD) --第一个参数是要搜索的值，第二个参数是搜索的类型，返回值是是否搜到数据列表，搜索类型为TYPE_AUTO，TYPE_BYTE，TYPE_DOUBLE，TYPE_DWORD，TYPE_FLOAT，TYPE_QWORD，TYPE_WORD，TYPE_XOR
7.将结果加载到结果列表中并将其作为表返回。
local r = gg.getResults(5)--获取列表前5项数据，返回到列表中，通过r[1].address，r[1].flags，r[1].value访问，地址是十进制数字
8.获取项列表的值。
local r = gg.getValues(table t)--可以配合gg.getResults(5)使用，通过r[1].address，r[1].flags，r[1].value访问
9.获取找到的结果数。
local c=gg.getResultsCount()
10.编辑所有搜索的结果
在调用此方法之前，必须通过getResults加载结果。值将仅应用于具有指定类型的结果。作用的是搜索结果集
editAll(string value, int type )
11.设置项列表的值。作用的是自定义列表集
setValues(table values)
如:
gg.searchNumber('10', gg.TYPE_DWORD)
local r = gg.getResults(5) -- load items
r[1].value = '15'
gg.setValues(r)
12.添加项到保存列表
gg.addListItems(t)--项有许多约束，可参考官网，最常用的如下
local t = {}
t[1] = {}
t[1].address =  0x18004030--十六进制
t[1].flags = gg.TYPE_FLOAT--数据类型
t[1].freeze = true--冻结，默认不冻结
t[1].value = "100"--值
gg.addListItems(t)--把列表添加到保存列表
13.根据地址获取数值
 gg.searchAddress("0xA1234567",0xFFFFFFFF,gg.TYPE_FLOAT)--地址可以是完整的地址也可以是某段'0B?0'，'A20'，如果不指定类型则搜索一个地址所有数据类型，地址必须是十六进制地址
14.休眠
gg.sleep(100)
15.其他
a.while循环
while true do--无限循环
    --循环体
    gg.sleep(100)
end
b.for循环
local t = {"a","b","c"}
for i, v in ipairs(t) do--ipairs是Lua提供的一个迭代器函数，用来迭代数组。
   print(v)--输出每一项的值
end
c.字符串转数字
local n = tonumber("123")
d.十进制转十六进制
local d = string.format("%#x","123456")--十进制数转十六进制地址有用
e.判断语句中等于是==，不等于是~=
f.条件判断
if 条件 then
    代码体
end--或者else继续判断
g.goto语句
::label::--定义标签
代码体
goto label--跳转到标签
16.UI浮动按钮
gg.showUiButton()--显示UI按钮
gg.hideUiButton()--隐藏UI按钮
gg.isClickedUiButton()--获取ui按钮的单击状态。如果点击返回真true