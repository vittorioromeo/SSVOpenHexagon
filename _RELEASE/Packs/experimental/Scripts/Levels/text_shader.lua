u_execDependencyScript("ohvrvanilla", "base", "vittorio romeo", "utils.lua")


function onInit()
	text_shader = shdr_getShaderId("text.frag")
	shdr_setActiveFragmentShader(RenderStage.TEXT, text_shader)

	-- make sure the text outline color isn't swapping
	s_setMaxSwapTime(0)
end


function onUpdate()
	shdr_setUniformFVec4(text_shader, "color0", s_getColor(0))
	shdr_setUniformF(text_shader, "time", l_getLevelTime())
end


function onLoad()
	e_messageAddImportant("test", 1000)
end
