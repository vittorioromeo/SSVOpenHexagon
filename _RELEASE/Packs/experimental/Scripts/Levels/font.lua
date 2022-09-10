function onLoad()
	e_messageAdd("This text is changing size!", 180)
	e_wait(180)
	e_eval("sizeChange = false")
	e_eval("u_setMessageFont(\"freesansbold.ttf\")")
	e_messageAdd("This text is using a custom font!", 200)
end

fontSize = 32
dir = 1
sizeChange = true
function onUpdate(frametime)
	if sizeChange then
		fontSize = fontSize + frametime * 2 * dir
		if fontSize > 100 then
			dir = -1
		end
		if fontSize < 10 then
			dir = 1
		end
	end
	u_setMessageFontSize(fontSize)
end
