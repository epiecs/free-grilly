# MQTT

Free-Grilly also supports sending and receiving data via a mqtt broker.

You can set your prefix, mqtt broker and port in the settings. The default prefix is **`free-grilly`**

The data being sent and received is in the json format. This is the same data that is sent and received by the rest api. For a in depth spec you can reference the [openapi](./openapi.yaml) file.

## Topics

The following topics are provided:

The default prefix is `free-grilly` and the uuid is in the format of a UUIDv4 generated on first boot.

- Publishing
    - `<prefix>/<uuid>/grill`
    - `<prefix>/<uuid>/probes`
    - `<prefix>/<uuid>/settings`
- Subscribing
    - `<prefix>/<uuid>/config/probes`
    - `<prefix>/<uuid>/config/settings`

A full example looks like `free-grilly/43c62ed2-4dc0-41a5-8f71-16db60155739/status`.

The `grill` topic is sent every second. The `probes` and `settings` topic is only used if you change your probe or your grill settings on free-grilly.

When publishing to the `probes` and `settings` topic the messages will have their **retain** flag set to true. That way the latest known settings are always available for whoever subscribes.

For the topics that Free-Grilly is subscribed to (`/config/probes` and `/config/settings`), Free-Grilly takes into account that there might be a message with the retain flag set. So after reading the message Free-Grilly will also publish a new message with retain set to true and a 0 byte payload to clear the existing message.
