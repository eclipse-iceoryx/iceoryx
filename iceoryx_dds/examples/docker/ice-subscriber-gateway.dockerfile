FROM iceoryx:latest

COPY ice-subscriber-gateway.entrypoint /root/
ENTRYPOINT ["/root/ice-subscriber-gateway.entrypoint"]
