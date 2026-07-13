#!/bin/bash

EVPN="/usr/local/bin/expressvpnctl"

case "$1" in
    toggle)
        state=$($EVPN get connectionstate 2>/dev/null)
        if [ "$state" = "Connected" ] || [ "$state" = "Connecting" ] || [ "$state" = "Reconnecting" ]; then
            $EVPN disconnect
        else
            $EVPN connect
        fi
        ;;
    change-region)
        regions=$($EVPN get regions 2>/dev/null | grep -v '^$')
        if [ -n "$regions" ]; then
            selected=$(echo "$regions" | rofi -dmenu -i -p "Select VPN Region")
            if [ -n "$selected" ]; then
                $EVPN connect "$selected"
            fi
        fi
        ;;
    *)
        state=$($EVPN get connectionstate 2>/dev/null)
        if [ "$state" = "Connected" ]; then
            region=$($EVPN get region 2>/dev/null)
            echo "{\"text\": \"󰖂 $region\", \"class\": \"connected\", \"tooltip\": \"ExpressVPN: Connected to $region\n\nLeft-click: Disconnect\nRight-click: Change region\"}"
        elif [ "$state" = "Connecting" ] || [ "$state" = "Reconnecting" ] || [ "$state" = "DisconnectingToReconnect" ] || [ "$state" = "Disconnecting" ]; then
            echo "{\"text\": \"󰖂 Connecting...\", \"class\": \"connecting\", \"tooltip\": \"ExpressVPN: Connecting/Changing...\n\nLeft-click: Disconnect\"}"
        else
            echo "{\"text\": \"󰗄 Disconnected\", \"class\": \"disconnected\", \"tooltip\": \"ExpressVPN: Disconnected\n\nLeft-click: Connect\nRight-click: Change region\"}"
        fi
        ;;
esac
