#include "i2c.h"

#define I2C_TIMEOUT  (0x4000)

//=============================================================================
// Funktion sendet einen Datenblock
//=============================================================================
int I2C_write_buf(	unsigned char slave_addr,
					unsigned char reg_addr,
					unsigned char length,
					unsigned char const *data)
{
	if (I2C_start(I2C1, slave_addr, I2C_Direction_Transmitter )) return 1;
	if (I2C_write(I2C1, reg_addr)) return 1;
	while (length--)
	{
		I2C_write(I2C1, *data++);
	}
	I2C_stop(I2C1 );
	return 0;
}

//=============================================================================
// Funktion empfängt einen Datenblock
//=============================================================================
int I2C_read_buf(	unsigned char slave_addr,
					unsigned char reg_addr,
					unsigned char length,
					unsigned char *data)
{
	if (I2C_start(I2C1, slave_addr, I2C_Direction_Transmitter )) return 1;
	if (I2C_write(I2C1, reg_addr)) return 1;
	// stop the transmission
	//I2C_stop(I2C1);
	if (I2C_restart(I2C1, slave_addr, I2C_Direction_Receiver )) return 1;

	while ((length--) > 1)
	{
		*data = I2C_read_ack(I2C1 );
		data++;
	}

	*data = I2C_read_nack(I2C1 );

	I2C_stop(I2C1 );

	return 0;
}


//=============================================================================
// Funktion sendet Start Condition + Slave Adresse und RW-Bit
//=============================================================================
int I2C_start(	I2C_TypeDef* I2Cx,
				uint8_t addr,
				uint8_t rdwr)
{
	uint32_t timeout = (100 * I2C_TIMEOUT);

	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY ))
	{
		if ((timeout--) == 0)
			return I2C_timeout("I2C_start(): bus busy\r\n");
	}
	return I2C_restart(I2Cx, addr, rdwr);
}


//=============================================================================
// Funktion sendet Stop Condition
//=============================================================================
void I2C_stop(	I2C_TypeDef* I2Cx)
{
	I2C_GenerateSTOP(I2Cx, ENABLE);
}


//=============================================================================
// Funktion sendet ein Byte an den I2C-Slave
//=============================================================================
int I2C_write(	I2C_TypeDef* I2Cx,
				uint8_t data)
{
	uint32_t timeout = I2C_TIMEOUT;
	I2C_SendData(I2Cx, data);
	// wait for I2Cx EV8_2 --> byte has been transmitted
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED ))
	{
		if ((timeout--) == 0)
		{
			I2C_stop(I2Cx);
			return I2C_timeout("I2C_write(): write byte failed\r\n");
		}
	}
	return 0;
}


//=============================================================================
// Funktion  empfängt ein Byte vom I2C-Slave und generiert ein ACK
//=============================================================================
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx)
{
	uint32_t timeout = I2C_TIMEOUT;
	I2C_AcknowledgeConfig(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED ))
	{
		if ((timeout--) == 0)
		{
			I2C_stop(I2Cx);
			return I2C_timeout("I2C_read_ack(): read byte failed\r\n");
		}
	}
	return I2C_ReceiveData(I2Cx);
}


//=============================================================================
// Funktion empfängt ein Byte vom I2C-Slave und generiert kein ACK
//=============================================================================
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx)
{
	uint32_t timeout = I2C_TIMEOUT;
	I2C_AcknowledgeConfig(I2Cx, DISABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED ))
	{
		if ((timeout--) == 0)
		{
			I2C_stop(I2Cx);
			return I2C_timeout("I2C_read_nack(): read byte failed\r\n");
		}
	}
	// read data from I2C data register and return data byte
	return I2C_ReceiveData(I2Cx);
}


//=============================================================================
// Funktion startet I2C neu
//=============================================================================
int I2C_restart(	I2C_TypeDef* I2Cx,
					uint8_t addr,
					uint8_t rdwr)
{
	uint32_t timeout = I2C_TIMEOUT;
	I2C_GenerateSTART(I2Cx, ENABLE);
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT ))
	{
		if ((timeout--) == 0)
		{
			I2C_stop(I2Cx);
			return I2C_timeout("I2C_start(): start failed\r\n");
		}
	}
	addr = addr << 1;
	I2C_Send7bitAddress(I2Cx, addr, rdwr);
	timeout = I2C_TIMEOUT;
	if (rdwr == I2C_Direction_Transmitter )
	{
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ))
		{
			if ((timeout--) == 0)
			{
				I2C_stop(I2Cx);
				return I2C_timeout("TxD I2C_start(): no ack for addr\r\n");
			}
		}
	}
	else if (rdwr == I2C_Direction_Receiver )
	{
		while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ))
		{
			if ((timeout--) == 0)
			{
				I2C_stop(I2Cx);
				return I2C_timeout("RxD I2C_start(): no ack for addr\r\n");
			}
		}
	}
	return 0;
}


//=============================================================================
// Funktion Testet ob Device vorhanden
//=============================================================================
int I2C_check_dev(	I2C_TypeDef* I2Cx,
					uint8_t addr)
{
	int timeout;
	timeout = I2C_TIMEOUT;
	while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY ))
	{
		if ((timeout--) == 0)
		{
			return 1;
		}
	}

	I2C_GenerateSTART(I2Cx, ENABLE);

	timeout = I2C_TIMEOUT;

	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT ))
	{
		if ((timeout--) == 0)  // wait while sending I2C-Bus START condition
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return 1;
		}
	}

	I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter );

	timeout = I2C_TIMEOUT;
	while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ))
	{
		if ((timeout--) == 0)   // wait while sending slave address for write
		{
			I2C_GenerateSTOP(I2Cx, ENABLE);
			return 1;
		}
	}

	I2C_GenerateSTOP(I2Cx, ENABLE);
	return 0;
}



int I2C_timeout(char *msg)
{
	char puffer[50];
	sprintf(puffer,"I2C Timeout: %s\n",msg);
	usart2_send(puffer);
	return 1;
}
