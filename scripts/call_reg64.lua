function onPre(v)
    vee.putreg(v, RAX, 64, 8)
    vee.putreg(v, RSI, 64, 0x50505050)
	for i=0,0x200 do
		vee.putmem(v, (0x0+(i*8)), 64, 0x40404040)
	end
end

function onPost(v)
    rip = vee.getreg(v, RIP, 64)
    rsi = vee.getreg(v, RSI, 64)

    if rip == nil then
        return false
    end

	if rip == 0x40404040  or rip == 0x50505050 then
        return true
    end

    return false
end

vee.register(onPre, onPost)
