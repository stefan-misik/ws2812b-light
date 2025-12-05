import animations


st = animations.AnimationStorage()
st.change(0)
an = st.get()
an.setParameter(65536, 2)
an.getParameter(65536)

pass