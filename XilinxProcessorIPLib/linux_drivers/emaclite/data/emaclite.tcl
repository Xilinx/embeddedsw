proc gen_mdio_node {drv_handle} {
    set mdio_child_name "mdio"
    set mdio [hsm::utils::add_new_child_node $drv_handle $mdio_child_name]
    hsm::utils::add_new_property $mdio "#address-cells" int 1
    hsm::utils::add_new_property  $mdio "#size-cells" int 0
    return $mdio
}

proc generate {drv_handle} {
    gen_mdio_node $drv_handle
}

