FROM iceoryx:latest

COPY ice-publisher-gateway.entrypoint /root/
ENTRYPOINT ["/root/ice-publisher-gateway.entrypoint"]
