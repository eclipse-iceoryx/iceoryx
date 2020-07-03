FROM iceoryx:latest

COPY ice_publisher_gateway.entrypoint /root/
ENTRYPOINT ["/root/ice_publisher_gateway.entrypoint"]
