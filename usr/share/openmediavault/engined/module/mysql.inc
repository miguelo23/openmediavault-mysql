<?php

/**
 * Copyright (C) 2010-2012 Ian Moore <imooreyahoo@gmail.com>
 * Copyright (C)      2013 OpenMediaVault Plugin Developers
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

require_once "openmediavault/module.inc";
require_once "openmediavault/config.inc";
require_once "openmediavault/error.inc";
require_once "openmediavault/util.inc";
require_once "openmediavault/initscript.inc";

class OMVModuleMySQL extends OMVModuleServiceAbstract implements OMVINotifyListener, OMVIModuleServiceStatus {

    private function getXPath()
    {
        return '//services/mysql';
    }
    
    private function getInitScriptName()
    {
        return 'mysql';
    }

	/**
	 * Get the module name.
	 */
	public function getName()
	{
		return "mysql";
	}

    /**
     * Get the module status
     */
    public function getStatus()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getXPath());

        $initd = new OMVSysVInitScript($this->getInitScriptName());

        return array(
            "name"    => $this->getName(),
            "title"   => gettext("MySQL"),
            "enabled" => boolval($object['enable']),
            "running" => ($initd->status() == 0)
        );
    }

	/**
	 * Create and update the configuration
	 */
	public function applyConfig()
	{
	    global $xmlConfig;

        $config = $xmlConfig->get($this->getXPath());

        if (is_null($config))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getXPath());

        $cmd = "sudo omv-mkconf mysql 2>&1";

        if (0 !== $this->exec($cmd, $output))
            throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
                $cmd, implode("\n", $output));
    }

    /**
     * Start the managed service
     */
    public function startService()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getXPath());

        if (boolval($object['enable']) === true) {
            $initd = new OMVSysVInitScript($this->getInitScriptName(), true);
            $initd->exec();
        }
    }

    /**
     * Stop the managed service
     */
    public function stopService()
    {
        global $xmlConfig;

        $object = $xmlConfig->get($this->getXPath());

        if (is_null($object))
            throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $this->getXPath());

        if (boolval($object['enable']) === false) {
            $initd = new OMVSysVInitScript($this->getInitScriptName(), false);
            $initd->exec();
        }
    }

	/**
	 * Bind listeners.
	 */
	public function bindListeners(OMVNotifyDispatcher $dispatcher) {
	    $dispatcher->addListener(
            OMV_NOTIFY_MODIFY,
            "org.openmediavault.services.mysql",
            array($this, "setDirty"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleMySQL());
