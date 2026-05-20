FILESEXTRAPATHS:prepend := "${THISDIR}/systemd-conf:"
 
SRC_URI:append = " \
    file://eth.network \
"
 
do_install:append() {
    install -d ${D}${sysconfdir}/systemd/network
    install -d ${D}${libdir}/systemd/network
    install -m 0644 ${WORKDIR}/eth.network ${D}${sysconfdir}/systemd/network/
    install -m 0644 ${WORKDIR}/eth.network ${D}${libdir}/systemd/network/80-wired.network
}
 
FILES:${PN}:append = " \
    ${sysconfdir}/systemd/network/* \
"

