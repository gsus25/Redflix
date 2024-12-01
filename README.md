# Redflix
Redflix es una plataforma de streaming diseñada para adaptarse al ancho de banda de la red de cada cliente, utilizando un sistema de bitrate adaptativo. Este sistema permite cambiar dinámicamente la calidad de video (HD, MD, LD) durante la transmisión. Está compuesto por tres módulos principales: Visor, Streamer, y Encoder.

### Ejecución del streamer
El Streamer se inicializa especificando el puerto y el archivo de video. Permite conexiones locales o remotas de múltiples Visores.

Para iniciar el servidor en un puerto específico:
```
$ ./streamer 8080 video.txt
Servidor escuchando en el puerto 8080.
```
Cuando un cliente se conecta y selecciona un bitrate:
```
Cliente 704 conectado en el socket 4.
Cliente 704 seleccionó HD.
Enviando frame "1" al cliente 704.
Enviando frame "2" al cliente 704.
```

### Ejecución del Visor
El Visor se conecta al Streamer mediante una IP y un puerto, permitiendo el control interactivo de la transmisión.

Para conectarse al servidor local:
```
$ ./visor 127.0.0.1 8080
```
Para conectarse a un servidor remoto:
```
$ ./visor <IP_DEL_SERVIDOR> 8080
```
El Visor permite al usuario enviar comandos:
```
Conectado al servidor.
Ingrese un comando (HD, MD, LD, STOP, PAUSE, PLAY): HD
```

### Comandos Básicos
- PLAY: Inicia la transmisión.
- PAUSE: Detiene temporalmente la transmisión.
- STOP: Finaliza la transmisión.
- HD, MD, LD: Cambia la calidad del video durante la transmisión.

### Compilación
Para compilar el programa:
```
$ make
```
Para compilar facilitando la depuración con gdb:
```
$ make debug
```
Para compilar habilitando la herramienta AddressSanitizer, facilita la depuración en tiempo de ejecución:
```
$ make sanitize
```
