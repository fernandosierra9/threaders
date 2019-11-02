## TP-2019-2C-Threaders
-----------------------------------------------------
#### Para correr el *client-tests*

1) Desde eclipse:

`Run > Run Configurations > Environment > New`

*Variables*

`Name: "LD_LIBRARY_PATH"`

`Value:
${PATH_PROYECTO}/hilolay/Debug:${PATH_PROYECTO}/libmuse/Debug:${PATH_PROYECTO}/sac_cli/Debug:${PATH_PROYECTO}/shared_common/Debug`

2) Desde terminal:

`export LD_LIBRARY_PATH=${PATH_PROYECTO}/hilolay/Debug:${PATH_PROYECTO}/libmuse/Debug:${PATH_PROYECTO}/sac_cli/Debug:${PATH_PROYECTO}/shared_common/Debug`

#### Para levantar FUSE dentro del *SAC_CLI*

_Mount Parameters_
------------------

El montaje de SAC_CLI utiliza la siguiente sintaxis:

./sac_cli.exe [OPTIONS] [MOUNTPOINT]

MOUNTPOINT es una carpeta vacía, en donde se montará el FS.

Además, la implementación de SAC_CLI permite la inclusión de varios parámetros de montaje. Ellos son:

--Disc-Path=[RUTA DISCO]	- Asigna la ruta donde se encuentra el disco.

Parámetros propios de FUSE:

-d 							- Abre el FileSystem en modo Debug.

-s 							- Abre el FileSystem en modo Single Thread.

-V							- Imprime la versión de FUSE que se está utilizando.

-o direct_io				- Deshabilita la cache


_Ejemplo_
---------

La sentencia:

    ./sac_cli.exe -d -o direct_io --Disc-Path=/home/utnso/disc.bin /tmp/fuse_test/
    
Monta el FileSystem en modo DEBUG, con la cache DESHABILITADA, en el punto de montaje "/tmp/fuse_test/", con los parámetros indicados.


_Desmontando el Disco_
-----------------------

Para desmontar el disco se utiliza el comando: 'fusermount' con el parametro -u, indicando la ruta de montaje del FileSystem

Por ejemplo: 
    fusermount -u /tmp/fusea

(siendo /tmp/fuse_test/ la ruta en donde se monto FUSE).
