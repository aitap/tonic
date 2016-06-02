all: tonic.ico tonic_64x64_rgba.c

convert = convert $(1) -resize $(2) -gravity center -extent $(2) $(4) $(3)
# $(1) = source file
# $(2) = target size
# $(3) = target filename
# $(4) = additional convert operations

tonic_%.png: src.svg
	$(call convert,src.svg,$*,$@,)

tonic.ico: tonic_16x16.png tonic_32x32.png tonic_48x48.png
	icotool -c -o tonic.ico $^

tonic_64x64_rgba.c: src.svg
	echo 'unsigned char tonic_64x64_rgba[] = {' > $@
	$(call convert,src.svg,64x64,rgba:-,-depth 8) | xxd -i >> $@
	echo '};' >> $@
