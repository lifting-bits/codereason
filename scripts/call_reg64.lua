function onPre(v)
--    vee.putreg(v, RAX, 64, 8)
    vee.putreg(v, RAX, 64, 0x50505050)
    vee.putreg(v, RBX, 64, 0x50505050)
    vee.putreg(v, RCX, 64, 0x50505050)
    vee.putreg(v, RDX, 64, 0x50505050)
    vee.putreg(v, RBP, 64, 0x50505050)
    vee.putreg(v, RSI, 64, 0x50505050)
    vee.putreg(v, RDI, 64, 0x50505050)
    vee.putreg(v, R8,  64, 0x50505050)
    vee.putreg(v, R9,  64, 0x50505050)
    vee.putreg(v, R10, 64, 0x50505050)
    vee.putreg(v, R11, 64, 0x50505050)
    vee.putreg(v, R12, 64, 0x50505050)
    vee.putreg(v, R13, 64, 0x50505050)
    vee.putreg(v, R14, 64, 0x50505050)
    vee.putreg(v, R15, 64, 0x50505050)
--	for i=0,0x200 do
--		vee.putmem(v, (0x0+(i*8)), 64, 0x40404040)
--	end
end

function onPost(v)
    rip = vee.getreg(v, RIP, 64)
--    rsi = vee.getreg(v, RSI, 64)

    if rip == nil then
        return false
    end

	if rip == 0x40404040  or rip == 0x50505050 then
        return true
    end

    return false
end

vee.register(onPre, onPost)
