function onPre(v)
    vee.putreg(v, RAX, 64, 50505050)
    vee.putreg(v, RBX, 64, 50505050)
    vee.putreg(v, RCX, 64, 50505050)
    vee.putreg(v, RDX, 64, 50505050)
    vee.putreg(v, RBP, 64, 50505050)
    vee.putreg(v, RSI, 64, 50505050)
    vee.putreg(v, RDI, 64, 50505050)
    vee.putreg(v, R8,  64, 50505050)
    vee.putreg(v, R9,  64, 50505050)
    vee.putreg(v, R10, 64, 50505050)
    vee.putreg(v, R11, 64, 50505050)
    vee.putreg(v, R12, 64, 50505050)
    vee.putreg(v, R13, 64, 50505050)
    vee.putreg(v, R14, 64, 50505050)
    vee.putreg(v, R15, 64, 50505050)
end

function onPost(v)
    rip = vee.getreg(v, RIP, 64)

    if rip == nil then
        return false
    end

	if rip == 50505050 and vee.getexit(v) == Call then
        return true
    end

    return false
end

vee.register(onPre, onPost)
