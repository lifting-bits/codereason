function onPre(v)
    vee.putreg(v, ESP, 32, 40000000)
    vee.putreg(v, EAX, 32, 80808080)
end

function onPost(v)
    esp = vee.getreg(v, ESP, 32)

    if esp == nil then
        return false
    end

    if esp > 80808080-16 and esp < 80808080+16 then
        if vee.hascalls(v) == false and vee.getexit(v) == Return then
            return true
        end
        return false
    end

    return false
end

vee.register(onPre, onPost)
