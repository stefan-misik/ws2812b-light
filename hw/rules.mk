KIKIT = kikit

.PHONY: panel fab clean

panel: $(PROJ)-panel.kicad_pcb

fab: $(PROJ)-panel.kicad_pcb
	$(KIKIT) fab jlcpcb --no-assembly --drc "$<" fab/

clean:
	$(RM) -r $(PROJ)-panel.kicad_* fab/


%-panel.kicad_pcb: %.kicad_pcb
	$(KIKIT) panelize \
	    --layout 'hspace: 2mm; vspace: 2mm; rows: 2' \
	    --tabs 'type: fixed; vcount: 2' \
	    --cuts 'type: mousebites; spacing: 1mm; offset: -0.25mm; prolong: -0.25mm' \
	    --framing 'type: railstb' \
	    --text 'type: simple; voffset: 2.5mm; width: 1mm; height: 1mm; thickness: 0.15mm; text: JLCJLCJLCJLC' \
	    --post 'millradius: 1mm' \
	    "$<" "$@"
