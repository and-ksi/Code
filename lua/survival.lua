gg.clearResults()
addr = {}
attack = 0
time = 0
point_addr=0
land = 0
land1 = 0
land2 = 0
noise = 0

gg.showUiButton()
while true do
	if gg.isClickedUiButton() then
		local ret = gg.choice({
			'初始化(需要人物属性上限及时间)',
			'修改及锁定人物属性',
			'攻击及消耗',
			'修改游戏时间',
			'修改噪音、精力、便意、洁净值'
		})

		if ret == 1 then
			::soso::

			local t=gg.prompt({"生命上限","体力上限","健康上限","饱食上限","水分上限","心情上限","大地图移速","day","小时","分钟"},
				{[1]=90,[2]=100,[3]=100,[4]=100,[5]=100,[6]=100,[7]=220,[8]=1,[9]=8,[10]=1},
				{[1]='number',[2]= 'number',[3]='number',[4]= 'number',[5]='number',[6]= 'number',[7]= 'number',[8]= 'number',[9]= 'number',[10]= 'number'})

			time = (t[8] - 1)*24*60 + t[9]*60 + t[10]
			local str = string.format("%dE;%dE;%dE;%dE;%dE;%dE;%dE;%dE~%dE::57",t[1],t[2],t[3],t[4],t[5],t[6],t[7],time-10,time+10)
			if gg.searchNumber(str, gg.TYPE_AUTO) then
				local c=gg.getResultsCount()
				if c==8 then
					local r=gg.getResults(8)
					r=gg.getValues(r)
					point_addr = r[7].address
					local i = 1
					for i = 1, 6, 1 do
						addr[i] = point_addr - (7 - i) * 8
					end

					land = 1

				else local ex = gg.alert('未找到，是否修改初始化值?', '是', nil, '否')
					if ex == 1 then
						goto soso
					end
					gg.alert("搜索到"..c.."项，请仔细查看人物属性后再次搜索!")
				end
			end
			gg.clearResults()
		end

		if ret == 2 then
			if land == 0 then
				gg.alert("请初始化！")
				goto soso
			end

			gg.alert("请勿超过属性上限！")
			local t=gg.prompt({"生命","体力","健康","饱食","水分","心情"},
				{[1]=90,[2]=90,[3]=90,[4]=90,[5]=90,[6]=90},
				{[1]='number',[2]= 'number',[3]='number',[4]= 'number',[5]='number',[6]= 'number'})
			local ex = gg.alert('是否将值全部冻结？','是', nil, '否')

			local data = {}
			local i
			for i = 1, 6, 1 do
				data[i] = {}
				data[i].address = addr[i]
				data[i].value = t[i]
				data[i].flags = gg.TYPE_DOUBLE
				if ex == 1 then
					data[i].freeze = true
				end
				gg.addListItems(data)
			end
			gg.setValues(data)
		end

		if ret == 3 then
			if land2 == 0 then
				local count = 0
				while count ~= 2 do
				gg.alert('请输入当前攻击力及攻击消耗,如重复询问，请变动相应值','是')
				local t = gg.prompt({"攻击力","攻击消耗"},{[1]=16,[2]=120},{[1]='number',[2]= 'number'})
				land2 = 1
				local str = string.format("%dD;%dD::16", t[1], t[2])
				gg.searchNumber(str, gg.TYPE_AUTO)
				count = gg.getResultsCount()
				end
					local r=gg.getResults(2)
					
					r=gg.getValues(r)
					attack = r[1].address
				
			end
			gg.alert('请输入目标攻击力及攻击消耗！','是')
			local t1 = gg.prompt({"攻击力","攻击消耗"},{[1]=3890,[2]=1},{[1]='number',[2]= 'number'})

			local att = {}
			att[1].value = t1[1]
			att[1].address = attack
			att[1].freeze = true
			att[1].flags = gg.TYPE_DWORD
			att[2].value = t1[2]
			att[2].address = attack + 16
			att[2].freeze = true
			att[2].flags = gg.TYPE_DWORD
			gg.addListItems(r)
			gg.setValues(r)
			
		end

		if ret == 4 then
			gg.alert('请输入需要增加的天数！','是')
			local t = gg.prompt({"天数"},{[1]=5000},{[1]='number'})
			local day = {}
			day[1] = {}
			day[1].address = point_addr + 8
			day[1].value = time + t[1]
			day[1].flags = gg.TYPE_DOUBLE
			gg.setValues(day)
		end

		if ret == 5 then
			if land1 == 0 then
				local count = 0
				while count ~= 1 do
					gg.alert("请输入当前噪音！")
					local t = gg.prompt({"噪音"},{[1]=16},{[1]='number'})
					land1 = 1
					local str = string.format("%dE~%dE", t[1]-1, t[1]+1)
					noise = point_addr + 24
					gg.searchNumber(str, gg.TYPE_AUTO, false, gg.SIGN_NOT_EQUAL, noise, noise + 8)
					count = gg.getResultsCount()
				end
				local r = gg.getResults(1)
				r = gg.getValues(r)
				noise = r[1].address
			end
				gg.alert("请输目标噪音！")
				local t = gg.prompt({"噪音"},{[1]=16},{[1]='number'})
				local noi = {}
				noi[1] = {}
				noi[1].flags = gg.TYPE_DOUBLE
				noi[1].address = noise
				noi[1].value = t[1]
				noi[1].freeze = true
				gg.addListItems(noi)
				gg.setValues(noi)
		end
	end
end