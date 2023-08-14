#ifndef IO_H
#define IO_H


#define readl(addr)             (*(volatile unsigned int *)(addr))
#define writel(val, addr)       (*(volatile unsigned int *)(addr) = (val))

#define readw(addr)             (*(volatile unsigned short *)(addr))
#define writew(val, addr)       (*(volatile unsigned short *)(addr) = (val))

#define readb(addr)             (*(volatile unsigned char *)(addr))
#define writeb(val, addr)       (*(volatile unsigned char *)(addr) = (val))

// --------------------------------------------------------------------
//	waiting to remove
// --------------------------------------------------------------------
#define ioread32(p)             (*(volatile unsigned int *)(p))
#define iowrite32(v, p)         (*(volatile unsigned int *)(p) = (v))

#define inl(p)                  ioread32(p)
#define outl(v, p)              iowrite32(v, p)

#define inw(port)               readl(port)
#define outw(port, val)         writel(val, port)

#define inb(port)               readb(port)
#define outb(port, val)         writeb(val, port)

#define inhw(port)              readw(port)
#define outhw(port, val)        writew(val, port)

#define masked_outw(port, val, mask)    outw(port, (inw(port) & ~mask) | (val & mask))

#define GET_BIT(port, __bit) \
    ((inw(port) & BIT##__bit) >> __bit)

#define GET_BITS(port, __s_bit, __e_bit) \
    ((inw(port) & (BIT##__e_bit | (BIT##__e_bit - BIT##__s_bit))) >> __s_bit)

#define SET_BIT(port, __bit) \
    outw(port, BIT##__bit)

#define SET_MASKED_BIT(port, val, __bit) \
    outw(port, (inw(port) & ~BIT##__bit) | ((val << __bit) & BIT##__bit))

#define SET_MASKED_BITS(port, val, __s_bit, __e_bit) \
    outw(port, ((inw(port) & ~(BIT##__e_bit | (BIT##__e_bit - BIT##__s_bit))) | (val << __s_bit))); 


#endif // IO_H
