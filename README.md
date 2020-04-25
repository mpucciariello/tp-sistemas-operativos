# tp-2020-1c-CDev20

Trabajo Practico: Delibird

##Integrantes:

Nombre y Apellido: Pablo Hervida
Legajo: 1473943
E-Mail: elpabli09@gmail.com

Nombre y Apellido: Derek Fernandez
Legajo: 1589027
E-Mail: derekfernandez182@gmail.com

Nombre y Apellido: Nicolas Taboada
Legajo: 1439388
E-Mail: ntaboada93@gmail.com

Nombre y Apellido: Fernando Sierra
Legajo: 1352404
E-Mail: fernandosierra9@gmail.com

Nombre y Apellido: Martin Yoo
Legajo: 1589635
E-Mail: mrtnyoo@gmail.com>


# formating-style-tp-so.xml

Archivo con el estilo para el formateo de codigo C

##Uso

Se debe importar desde el Eclipse en la seccion formatting style del editor y seleccionar donde se encuentre el archivo

# so-deploy

so-deploy es una herramienta para facilitar el proceso de deploy de los proyectos.

## Uso

Al ejecutar el script, se deben pasar los siguientes parametros:
* -t | --target: Cambia el directorio donde sera ejecutado el script. Por defecto se ejecuta en el directorio actual.
* -l | --lib: Agrega una dependencia externa para compilar e instalar. (Se especifica user y nombre del repositorio en github)
* -d | --dependency: Agrega una dependencia interna del proyecto para compilar e instalar. (Forman parte del repositorio a deployar y se especifica una ruta dentro del repositorio a donde reside la dependencia).
* -p | --proyect: Agrega un proyecto a compilar del repositorio. (Al igual que las dependencias se puede pasar una ruta a los proyectos).

Para ver informacion de como usarlo, ejecutar con la opcion -h (help) `./deploy.sh -h`

### Ejemplo

`deploy.sh -d=shared-common -p=broker -p=game-card -p=team -p=game-boy tp-2020-1c-CDev20`

### Requerimientos

so-deploy requiere que los proyectos y dependencias tengan un makefile encargado de compilar correctamente a cada uno.
La estructura debe ser la siguiente:

```
repo
│  
└─── Proyecto/  
|     └─── makefile  
└─── Dependencia/  
      └─── makefile
```


export LD_LIBRARY_PATH=/{PATH_PROYECTO}/shared-common/Debug