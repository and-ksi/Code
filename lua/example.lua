gg.clearResults()--清除数据
dengji,jifen="",""--等级，积分地址
shiju=""--视距
gongjijuli=""--攻击距离
zong,dangqian="",""--总生命中，当前生命中
--显示浮动按钮
gg.showUiButton()
--无限循环判断触发事件
while true do
    --按钮点击事件
    if gg.isClickedUiButton() then
        --调用操作选项
       local ret = gg.choice(
          {'等级和积分',
           '视距', 
           '生命值', 
           '攻击距离',
           "修改",
           '退出'})
        if ret == 1 then--等级积分
         ::soso::--定义标签
         dengji,jifen="",""--初始化
          local t=gg.prompt({"等级","积分"},
          {[1]=1},
          {[1]='number',[2]= 'number'})
          if t == nil then
              --gg.alert('取消返回!')
          else
            if t[1]=="" or tonumber(t[1])<=0 then
                gg.alert("等级不能空!")
            else
            if t[2]=="" or tonumber(t[2])<=0 then
                gg.alert("积分不能为空!")
            else
            --输入成功
                 --gg.clearResults()
                 local str=t[1].."D;1D;25D;"..t[2].."D;0D::24"--联合搜索字符串
                 --str="560"--测试
                 --gg.clearResults()--清除数据
                 if gg.searchNumber(str, gg.TYPE_DWORD) then
                     local c=gg.getResultsCount()
                     if c==5 then
                     --找到了，提取等级和积分地址
                     local r=gg.getResults(5)
                     r=gg.getValues(r)
                     dengji=r[1].address
                     jifen=r[4].address
                     --把需要的项写入表格并保存到列表
                     local t = {}
                     t[1] = {}
                     t[1].address =  dengji
                     t[1].flags = gg.TYPE_DWORD
                     --t[1].freeze = true--冻结
                     t[1].value = r[1].value
                     t[2] = {}
                     t[2].address =  jifen
                     t[2].flags = gg.TYPE_DWORD
                     --t[2].freeze = true--冻结
                     t[2].value = r[4].value
                     gg.addListItems(t)--把列表添加到保存列表
                     --gg.setValues(t)--修改列表值
                     --找到地址添加成功清楚数据
                     gg.clearResults()--清除数据
                     else
                     --[[
                         local ex = gg.alert('未找到，是否再次搜索?', '是', nil, '否')
                         if ex == 1 then--返回1为"是"  3为"否"
                            goto soso--跳转到标签
                          end
                          ]]
                          gg.alert("搜索到"..c.."项，请增加积分或者改变等级后再次搜索!")
                     end
                 end
            end
          end
        end
        end
        if ret == 2 then--视距"
        gg.clearResults()--清除数据
        shiju=""
            if gg.searchNumber("22.0F;3D;0D;27.5F;33.0F;3.0F::24", gg.TYPE_FLOAT) then
                     local c=gg.getResultsCount()
                     if c==6 then
                     --找到了，提取视野地址
                     local r=gg.getResults(6)
                     r=gg.getValues(r)
                     shiju=r[1].address
                     --把需要的项写入表格并保存到列表
                     local t = {}
                     t[1] = {}
                     t[1].address =  shiju
                     t[1].flags = gg.TYPE_FLOAT
                     --t[1].freeze = true--冻结
                     t[1].value = r[1].value
                     gg.addListItems(t)--把列表添加到保存列表
                     --gg.setValues(t)--修改列表值
                     --找到地址添加成功清楚数据
                     gg.clearResults()--清除数据
                     else
                          gg.alert("搜索到"..c.."项，请手动修改视距!")
                     end
                 end
        end
        if ret == 3 then--生命值
        zong=""
        dangqian=""
          local t=gg.prompt({"总生命值","当前生命值"},
          {[1]=5,[2]=5},
          {[1]='number',[2]= 'number'})
          if t == nil then
              --gg.alert('取消返回!')
          else
            if t[1]=="" or tonumber(t[1])<=0 then
                gg.alert("总生命值不能空!")
            else
            if t[2]=="" or tonumber(t[2])<=0 then
                gg.alert("当前生命值不能为空!")
            else
            local str=(tonumber(t[1])*25).."F;"..(tonumber(t[2])*25).."F;0D;1D::16"
            if gg.searchNumber(str, gg.TYPE_FLOAT) then
                     local c=gg.getResultsCount()
                     if c==4 then
                     --找到了，提取攻击距离地址
                     local r=gg.getResults(4)
                     r=gg.getValues(r)
                     zong=r[1].address
                     dangqian=r[2].address
                     --把需要的项写入表格并保存到列表
                     local t = {}
                     t[1] = {}
                     t[1].address =  zong
                     t[1].flags = gg.TYPE_FLOAT
                     --t[1].freeze = true--冻结
                     t[1].value = r[1].value
                     t[2] = {}
                     t[2].address =  dangqian
                     t[2].flags = gg.TYPE_FLOAT
                     t[2].freeze = true--冻结
                     t[2].value = r[2].value
                     gg.addListItems(t)--把列表添加到保存列表
                     --gg.setValues(t)--修改列表值
                     --找到地址添加成功清楚数据
                     gg.clearResults()--清除数据
                     else
                          gg.alert("搜索到"..c.."项，请手动修改生命值!")
                     end
                 end
              end
        end
        end
        end
        if ret == 4 then--攻击距离
            gg.clearResults()--清除数据
            gongjijuli=""
            if gg.searchNumber("0.5F;30.0F;6.0F;25.0F::16", gg.TYPE_FLOAT) then
                     local c=gg.getResultsCount()
                     if c==4 then
                     --找到了，提取攻击距离地址
                     local r=gg.getResults(4)
                     r=gg.getValues(r)
                     gongjijuli=r[3].address
                     --把需要的项写入表格并保存到列表
                     local t = {}
                     t[1] = {}
                     t[1].address =  gongjijuli
                     t[1].flags = gg.TYPE_FLOAT
                     --t[1].freeze = true--冻结
                     t[1].value = r[3].value
                     gg.addListItems(t)--把列表添加到保存列表
                     --gg.setValues(t)--修改列表值
                     --找到地址添加成功清楚数据
                     gg.clearResults()--清除数据
                     else
                          gg.alert("搜索到"..c.."项，请手动修改攻击距离!")
                     end
                 end
        end
        if ret == 5 then--其他选项
            local td1,td2,td3={},{},{}
            local ads,tys,vls={},{},{}
            --dengji="111"
           -- jifen="555"
            local i=1
            if dengji~="" then
                td1[i]="等级"
                gg.clearResults()--清除数据
                --根据地址获取数值
                if gg.searchAddress(string.format("%#x",dengji),0xFFFFFFFF,gg.TYPE_DWORD) then
                   local r=gg.getResults(1)
                   r=gg.getValues(r)
                   td2[i]=r[1].value
               end
                --td2[i]=""
                td3[i]="number"
                ads[i]=dengji
                tys[i]=gg.TYPE_DWORD
                vls[i]=false--是否转换
                i=i+1
            end
            if jifen~="" then
                td1[i]="积分"
                gg.clearResults()--清除数据
                --根据地址获取数值
                if gg.searchAddress(string.format("%#x",jifen),0xFFFFFFFF,gg.TYPE_DWORD) then
                   local r=gg.getResults(1)
                   r=gg.getValues(r)
                   td2[i]=r[1].value
               end
              --  td2[i]=""
                td3[i]="number"
                ads[i]=jifen
                tys[i]=gg.TYPE_DWORD
                vls[i]=false--是否转换
                i=i+1
            end
            if shiju~="" then
                td1[i]="视距"
                gg.clearResults()--清除数据
                --根据地址获取数值
                if gg.searchAddress(string.format("%#x",shiju),0xFFFFFFFF,gg.TYPE_FLOAT) then
                   local r=gg.getResults(1)
                   r=gg.getValues(r)
                   td2[i]=r[1].value
               end
               -- td2[i]=""
                td3[i]="number"
                ads[i]=shiju--地址
                tys[i]=gg.TYPE_FLOAT--数据类型
                vls[i]=false--是否转换
                i=i+1
            end
            if gongjijuli~="" then
                td1[i]="攻击距离"
                gg.clearResults()--清除数据
                --根据地址获取数值
                if gg.searchAddress(string.format("%#x",gongjijuli),0xFFFFFFFF,gg.TYPE_FLOAT) then
                   local r=gg.getResults(1)
                   r=gg.getValues(r)
                   td2[i]=r[1].value
               end
               -- td2[i]=""
                td3[i]="number"
                ads[i]=gongjijuli
                tys[i]=gg.TYPE_FLOAT
                vls[i]=false--是否转换
                i=i+1
            end
            if zong~="" then
                td1[i]="总生命值"
                gg.clearResults()--清除数据
                --根据地址获取数值
                if gg.searchAddress(string.format("%#x",zong),0xFFFFFFFF,gg.TYPE_FLOAT) then
                   local r=gg.getResults(1)
                   r=gg.getValues(r)
                   td2[i]=tonumber(r[1].value)/25
               end
                --td2[i]=""
                td3[i]="number"
                ads[i]=zong
                tys[i]=gg.TYPE_FLOAT
               vls[i]=true--是否转换
                i=i+1
            end
            if dangqian~="" then
                td1[i]="当前生命值"
                gg.clearResults()--清除数据
                --根据地址获取数值
                if gg.searchAddress(string.format("%#x",dangqian),0xFFFFFFFF,gg.TYPE_FLOAT) then
                   local r=gg.getResults(1)
                   r=gg.getValues(r)
                   td2[i]=tonumber(r[1].value)/25
               end
            --    td2[i]=""
                td3[i]="number"
                ads[i]=dangqian
                tys[i]=gg.TYPE_FLOAT
                vls[i]=true--是否转换
                i=i+1
            end
           local t=gg.prompt(td1,td2,td3)
          if t ~= nil then
          local tt={}
          --使用for循环遍历
          --i是数组索引值，v是对应索引的数组元素值。
          --ipairs是Lua提供的一个迭代器函数，用来迭代数组。
               for i,v in ipairs(t) do
                   tt[i]={}
                   if vls[i] then--转换
                      tt[i].value=tonumber(v)*25
                   else
                      tt[i].value=v
                   end
                   tt[i].address=ads[i]
                   tt[i].flags=tys[i]
               end
               --修改列表里的数据
              gg.setValues(tt)
          end
        end
        if ret == 6 then--退出
            local ex = gg.alert('你是否要退出脚本?', '是', nil, '否')
            if ex == 1 then--返回1为"是"  3为"否"
                 os.exit()--退出脚本
            end
        end
    end
    gg.sleep(100)
end