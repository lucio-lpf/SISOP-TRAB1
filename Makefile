#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#
all: cthread.o
	ar crs lib/libcthread.a bin/cthread.o bin/support.o
cthread.o:
	gcc -c src/cthread.c -o bin/cthread.o
clean:
	rm bin/cthread.o lib/libcthread.a
