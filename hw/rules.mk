KIKIT = kikit

.PHONY: panel clean

panel: $(PROJ)-panel.kicad_pcb

clean:
	$(RM) $(PROJ)-panel.kicad_*


%-panel.kicad_pcb: %.kicad_pcb
	$(KIKIT) panelize \
	    --layout 'hspace: 2mm; vspace: 2mm; rows: 2' \
	    --tabs 'type: fixed; vcount: 2' \
	    --cuts 'type: mousebites; spacing: 1mm; offset: -0.25mm; prolong: -0.25mm' \
	    --framing 'type: railstb' \
	    --text 'type: simple; voffset: 2.5mm; width: 1mm; height: 1mm; thickness: 0.15mm; text: JLCJLCJLCJLC' \
	    --post 'millradius: 1mm' \
	    "$<" "$@"
