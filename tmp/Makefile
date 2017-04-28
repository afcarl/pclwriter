# Target     
PROGRAM = pclwriter.node

LIBS = "../lib/"  
    
NODE_LIB = node.lib  
    
node = "../include/"    
inc = "../inc/"    
src = "../src/"      
INCLUDEDIRS =  /I $(inc) /I $(node) /I $(LIBS)
   
# Flags    
CPPOPT = $(INCLUDEDIRS) /w /EHsc /D_CRT_SECURE_NO_DEPRECATE    
    
# Compiler     
cc = cl     
CFLAGS =     
     
LFLAGS = /LIBPATH:$(LIBS)     
     
# list of source files     
CPPSOURCES =  addon.cc  ypclwriter.cc 
    
# expands to list of object files            
CPPOBJECTS = $(CPPSOURCES:.cc=.obj)     
      
all: $(PROGRAM)    
    
$(PROGRAM): $(CPPOBJECTS)    
    link.exe /out:../build/Release/$(PROGRAM) /DLL  $(CPPOBJECTS) $(LFLAGS) $(NODE_LIB)     
        
addon.obj:     
    $(cc) $(CPPOPT) /c ../src/addon.cc         
  
ypclwriter.obj:     
    $(cc) $(CPPOPT) /c ../src/ypclwriter.cc         
      
       
            
clean:      
    del $(CPPOBJECTS) $(PROGRAM)          