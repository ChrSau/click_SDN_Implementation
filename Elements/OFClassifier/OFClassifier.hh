#ifndef CLICK_OFCLASSIFIER__HH
#define CLICK_OFCLASSIFIER__HH 1

#include <click/config.h>
#include <click/element.hh>
#include <click/args.hh>

#include "../Utilities/openflow.h"

#include "../Utilities/OFFunctions.hh"

CLICK_DECLS

/*
=c

OFClassifier(DB)

=s

Sortiert eingehende Nachrichten vom Socket. Nachrichten fuer den Responder werden an den Ausgang 0 geleitet, Nachrichten für die Pipeline an den Ausgang 1.

=d

---

*/

class OFClassifier : public Element
{
private:
    int _DBlevel = -1;

public:
    OFClassifier(/* args */);
    OFClassifier(OFClassifier *&in);
    ~OFClassifier();

    const char *class_name() const { return "OFClassifier"; }
    const char *port_count() const { return "1/2"; }
    const char *processing() const { return PUSH; }
    const char *flow_code() { return "x/x"; }

    virtual void cleanup(CleanupStage) CLICK_COLD;

    int initialize(ErrorHandler *errh);
    int configure(Vector<String> &conf, ErrorHandler *errh);
    void push(int, Packet *p);
};

CLICK_ENDDECLS

#endif