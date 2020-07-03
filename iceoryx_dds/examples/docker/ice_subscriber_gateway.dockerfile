FROM iceoryx:latest

COPY ice_subscriber_gateway.entrypoint /root/
ENTRYPOINT ["/root/ice_subscriber_gateway.entrypoint"]
