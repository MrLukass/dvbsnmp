/* 
 * File:   config.h
 * Author: lukas
 *
 * Created on January 17, 2015, 9:37 AM
 */

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FRONTENDDEVICE "/dev/dvb/adapter%d/frontend%d"
#define MAXDEVICECOUNT 8
#define INPUTXMLDOCUMENTPATH "/tmp/dvbinfoout.xml"
#define LONGCACHETIMEOUT 30
#define SHORTCACHETIMEOUT 5

    



#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

