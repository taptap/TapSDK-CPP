file(GLOB CSHARP_FILES ${CSHARP_SRC_DIR}/*.cs)
list(JOIN CSHARP_FILES "\n" CSHARP_LST_CONTENT)
file(WRITE ${CSHARP_LST} ${CSHARP_LST_CONTENT})
