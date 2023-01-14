#ifndef IGMP_H_INCLUDED
#define IGMP_H_INCLUDED

uint16_t ip_check_sum(uint16_t *buf, uint8_t len);
int igmpSeng(uint8_t messageType);



#endif // IGMP_H_INCLUDED
