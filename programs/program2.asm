; Test program for SUB instruction

START:
    MOV RAX, 10
    MOV RCX, 3
    SUB RAX, RCX  ; RAX should be 7

    MOV RBX, 5
    SUB RBX, 2    ; RBX should be 3

EXIT:
