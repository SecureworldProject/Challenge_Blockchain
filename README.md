# Challenge_Blockchain
Challenge que comprueba si existe una conexión actual con Secureworld blockchain

Ejemplo de configuracion json
```json
{
	"FileName": "blockchain_challenge.dll",
	"Description": "Challenge that checks if there is a current connection with Secureworld blockchain",
	"Props": {
		"validity_time": 3600,
		"refresh_time": 3000
	},
	"Requirements": "none"
}
```

## Funcionamiento

Se hace uso del comando ping del sistema, lanzado desde C. El challenge lanza un ping a una dirección IP donde se encuentra uno de los nodos de la blockchain. La respuesta es un booleano (0 no, 1 sí) y constituye la clave del challenge.


