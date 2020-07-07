#include "broker_config.h"

void read_config(t_config* config_file);

int broker_config_load()
{
	return config_load(NULL, CONFIG_FILE_PATH, read_config, broker_print_config);
}

e_memory_struct broker_algoritmo_memoria_from_string(char* algoritmo)
{
	if(string_equals_ignore_case(algoritmo, "bs"))
	{
		return BS;
	}
	else if(string_equals_ignore_case(algoritmo, "pd"))
	{
		return PD;
	}
	else
	{
		return -1;
	}
}

e_algoritmo_reemplazo broker_algoritmo_reemplazo_from_string(char* algoritmo)
{
	if(string_equals_ignore_case(algoritmo, "fifo"))
	{
		return FIFO;
	}
	else if(string_equals_ignore_case(algoritmo, "lru"))
	{
		return LRU;
	}
	else
	{
		return -1;
	}
}

e_algoritmo_particion_libre broker_algoritmo_particion_libre_from_string(char* algoritmo)
{
	if(string_equals_ignore_case(algoritmo, "ff"))
	{
		return FF;
	}
	else if(string_equals_ignore_case(algoritmo, "bf"))
	{
		return BF;
	}
	else
	{
		return -1;
	}
}

void broker_config_free()
{
	free(broker_config->ip_broker);
	free(broker_config->log_file);
	free(broker_config);
}

void read_config(t_config* config_file)
{
	broker_config = malloc(sizeof(t_broker_config));
	broker_config->tamano_memoria = config_get_int_value(config_file, "TAMANO_MEMORIA");
	broker_config->tamano_minimo_particion = config_get_int_value(config_file, "TAMANO_MINIMO_PARTICION");
	broker_config->estrategia_memoria = broker_algoritmo_memoria_from_string(config_get_string_value(config_file, "ALGORITMO_MEMORIA"));
	broker_config->algoritmo_reemplazo = broker_algoritmo_reemplazo_from_string(config_get_string_value(config_file, "ALGORITMO_REEMPLAZO"));
	broker_config->algoritmo_particion_libre = broker_algoritmo_particion_libre_from_string(config_get_string_value(config_file, "ALGORITMO_PARTICION_LIBRE"));
	broker_config->ip_broker = malloc(sizeof(char*));
	broker_config->ip_broker = string_duplicate(config_get_string_value(config_file, "IP_BROKER"));
	broker_config->puerto_broker = config_get_int_value(config_file, "PUERTO_BROKER");
	broker_config->frecuencia_compactacion = config_get_int_value(config_file, "FRECUENCIA_COMPACTACION");
	broker_config->log_file = malloc(sizeof(char*));
	broker_config->log_file = string_duplicate(config_get_string_value(config_file, "LOG_FILE"));

}

char* broker_algoritmo_memoria_to_string(e_memory_struct algoritmo)
{
	switch (algoritmo)
	{
		case BS:
			return BUDDY_SYSTEM;
			break;
		case PD:
			return PARTICION_DINAMICA;
			break;
		default:
			return "";
			break;
	}
}

char* broker_algoritmo_reemplazo_to_string(e_algoritmo_reemplazo algoritmo)
{
	switch (algoritmo)
	{
		case FIFO:
			return FIFO_STRING;
			break;
		case LRU:
			return LRU_STRING;
			break;
		default:
			return "";
			break;
	}
}

char* broker_algoritmo_particion_libre_to_string(e_algoritmo_particion_libre algoritmo)
{
	switch (algoritmo)
	{
		case FF:
			return FIRST_FIT;
			break;
		case BF:
			return BEST_FIT;
			break;
		default:
			return "";
			break;
	}
}

void broker_print_config()
{
	broker_logger_info("TAMANO_MEMORIA: %d", broker_config->tamano_memoria);
	broker_logger_info("TAMANO_MINIMO_PARTICION: %d", broker_config->tamano_minimo_particion);
	broker_logger_info("ALGORITMO_MEMORIA: %s", broker_algoritmo_memoria_to_string(broker_config->estrategia_memoria));
	broker_logger_info("ALGORITMO_REEMPLAZO: %s", broker_algoritmo_reemplazo_to_string(broker_config->algoritmo_reemplazo));
	broker_logger_info("ALGORITMO_PARTICION_LIBRE: %s", broker_algoritmo_particion_libre_to_string(broker_config->algoritmo_particion_libre));
	broker_logger_info("IP_BROKER: %s", broker_config->ip_broker);
	broker_logger_info("PUERTO_BROKER: %d", broker_config->puerto_broker);
	broker_logger_info("FRECUENCIA_COMPACTACION: %d", broker_config->frecuencia_compactacion);
	broker_logger_info("LOG_FILE: %s", broker_config->log_file);
}
