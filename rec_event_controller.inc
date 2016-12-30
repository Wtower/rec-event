<?php
/**
 * @file
 *   ${entity_name} Entity Controller
 * 
 * @author
 *   gkarak@forecasting.gr
 *   
 * @copyright
 *   (c) 2014 George Karakostas
 */

class RecEventController 
    extends EntityAPIController 
{
  /**
   * Create entity
   * 
   * Specify default values, otherwise all others will be handled by 
   * form defaults
   * 
   * Called automatically or by .module entity_create()
   * If values are specified by entity_create, the merge will keep 
   * previous values
   * @see http://www.php.net/manual/en/language.operators.array.php
   * 
   * @param array $values
   * @return
   */
  public function create(array $values = array()) {
    global $user;
    $values += array(
      'title' => '',
      'description' => '',
      'recurrence_type' => 0,
      'recurrence_subtype' => 0,
      'recurrence_edit_status' => 0,
      'revision' => -1,
      'created_date' => REQUEST_TIME,
      'modified_date' => REQUEST_TIME,
      'created_uid' => $user->uid,
      'modified_uid' => $user->uid,
    );
    return parent::create($values);
  }

  /**
   * Save entity
   * 
   * @param $entity
   * @param DatabaseTransaction $transaction
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    //debug('Entering RecEventController::save()');
    //debug(field_info_field('node_reference'));
    //debug(field_info_instance('rec_event', 'node_reference', 'working'));
    //debug(field_info_field('event_date'));
    //debug(field_info_instance('rec_event', 'event_date', 'working'));
    
    // serialize week days if an array
    //debug(serialize($form_state['values']['recurrence_weekly_days']));
    // 'a:7:{i:1;s:1:"1";i:2;s:1:"2";i:3;s:1:"3";i:4;s:1:"4";i:5;s:1:"5";i:6;s:1:"6";i:7;s:1:"7";}' LEN 92
    // 'a:7:{i:1;i:0;i:2;i:0;i:3;i:0;i:4;i:0;i:5;i:0;i:6;i:0;i:7;i:0;}'
    if (isset($entity->recurrence_weekly_days) && 
        is_array($entity->recurrence_weekly_days)) 
    {
      $entity->recurrence_weekly_days = 
        serialize($entity->recurrence_weekly_days);
    }
    // Handle max date
    if (isset($entity->recurrence_max_date)) {
      if ($entity->recurrence_max_date == '0') 
        unset($entity->recurrence_max_date);
      elseif (is_string($entity->recurrence_max_date)) {
        $date = new DateObject($entity->recurrence_max_date);
        $entity->recurrence_max_date = $date->getTimestamp();
      }
    }
    // failsafe from buildRecurrence
    /*if (isset($entity->parent_event_id) && isset($entity->event_id) &&
        ($entity->parent_event_id == $entity->event_id)) 
    {
      //$entity->parent_event_id = NULL;
    }*/
    // meta
    $entity->revision++;
    $entity->modified_date = REQUEST_TIME;
    global $user;
    $entity->modified_uid = $user->uid;
    
    // save
    //return parent::save($entity, $transaction);
    $return = parent::save($entity, $transaction);
    // we expect SAVED_NEW (1) or SAVED_UPDATED (2)
    // @todo define return codes
    // @see http://drupalcontrib.org/api/drupal/drupal%21includes%21common.inc/function/drupal_write_record/7

    // Build recurrence
    if ($this->buildRecurrence($entity) == 2) {
      // if flagged then resave
      $return = parent::save($entity, $transaction);
    }
    return $return;
  }
  
  /**
   * Build recurrence
   * 
   * This will be invoked for each entity save, be careful for recursions:
   * Parent Event ID should never be set outside this class
   * See relevant dev note. 
   * @see http://stackoverflow.com/questions/8236354/php-is-null-or-empty
   * 
   * To debug you can disable entity save
   * 
   * Using DateTime class (rev2)
   * @see http://php.net/manual/en/class.datetime.php
   * @see http://www.php.net/manual/en/language.operators.comparison.php
   * @see http://www.php.net/manual/en/datetime.formats.date.php
   * @see http://www.php.net/manual/en/function.date.php
   * @see http://www.php.net/manual/en/datetime.gettimestamp.php
   * @see http://www.php.net/manual/en/datetime.settimestamp.php
   * @see http://www.php.net/manual/en/datetime.add.php
   * @see http://www.php.net/manual/en/datetime.sub.php
   * @see http://www.php.net/manual/en/datetime.diff.php
   * @see http://www.php.net/manual/en/class.dateinterval.php
   * @see https://en.wikipedia.org/wiki/ISO_8601
   * @see http://stackoverflow.com/questions/1995562/now-function-in-php
   * 
   * Using Date module Date API
   * @see http://drupalcontrib.org/api/drupal/contributions%21date%21date_api%21date_api.module/7
   * @see https://drupal.org/node/1868064
   * 
   * Return values
   * 0: no recurrence / published type / new recurring non-parent / exception / 
   *     edit all but non-parent: do nothing for this event
   * 1: recurrence built, $entity is ok
   * 2: recurrence built, $entity needs resave
   * 
   * @param $entity
   * @return int
   */
  public function buildRecurrence(&$entity) {
    /*
     * Validations
     */
    $r = 0;
    // not recurring
    if ($entity->recurrence_type == 0) return $r;
    // is published type
    $type = rec_event_type_load($entity->type);
    if ($type->state > 0) return $r;
    // just in case set NEW
    if (!isset($entity->recurrence_edit_status))
      $entity->recurrence_edit_status = 0;
    // First initialisations
    $wrapper = entity_metadata_wrapper('rec_event', $entity);
    $parent_date = $wrapper->event_date->value(); 
    $date_pos = new DateObject($parent_date['value']);
    $date_pos_end = new DateObject($parent_date['value2']);
    $duration = $date_pos->diff($date_pos_end);
    $now = new DateObject('now');
    // is parent? NO
    if (isset($entity->parent_event_id) && !empty($entity->parent_event_id)) {
      // if NEW (comes from auto-creation procedure) OR EXCEPTION? return
      if ($entity->recurrence_edit_status < 2) return $r;
      // if NEXT? then 
      // set parent event end date to this event date, no repopulate
      // then proceed to LOOP-THROUGH (update-create/delete)
      //
      if ($entity->recurrence_edit_status == 2) {
        // first set parent id=this id for all events 
        // where parentid=old parentid and 
        // date >= this date (or ID, is sequential)
        // otherwise saving new max_date to parent entity 
        // will delete the remaining events!
        // http://drupal.stackexchange.com/questions/108188/is-it-proper-to-perform-a-db-update-on-an-entity-table
        // @see Drupal.odt
        // db_update is safe: entities will be re-saved through API
        try {
          db_update('rec_event')
            ->fields(array(
                'parent_event_id' => $entity->event_id
            ))
            ->condition('parent_event_id', $entity->parent_event_id)
            ->condition('event_id', $entity->event_id, '>=') 
            // this will change this controller event too but it is ok 
            // since it will be properly saved by API when we'll return 2.
            ->execute();
        }
        catch (Exception $e) {
          drupal_set_message(
              t('Update failed. Message = %message, query = %query',
              array(
                  '%message' => $e->getMessage(), 
                  '%query' => $e->query_string,
              )), 'error');
        }
        // Get the number of occurences prior to this edit 
        // in order to set max_occ for old parent
        $select = db_select('rec_event', 'e')
          ->fields('e', array('event_id'))
          ->condition('parent_event_id', $entity->parent_event_id)
          ->orderBy('event_id');
        $ids = $select->execute()->fetchCol();
        // move max_date and max_occ to old parent, now is safe to repopulate 
        // without affecting old remains
        $entity_parent = rec_event_load($entity->parent_event_id);
        // if for some reason the parent no longer exists 
        // (false returned - data discrepancy), then we are ok
        if ($entity_parent) {
          $entity_parent->recurrence_max_date = $date_pos->getTimestamp();
          $entity_parent->recurrence_max_occ = count($ids) + 1;
          $entity_parent->recurrence_edit_status = 3;
          rec_event_save($entity_parent);
        }
        // now set this parent id to null
        $r = 2;
        $entity->parent_event_id = NULL;
        // now is ok to LOOP-THROUGH baby
      }
      // if ALL? then
      // edit parent with the details of this event and disregard this 
      else {
        $entity_parent = rec_event_load($entity->parent_event_id);
        if ($entity_parent) {
          RecEventController::copy($entity, $entity_parent);
          // reset from copy
          $entity_parent->parent_event_id = NULL;
          // set explicitly: won't be copied
          $entity_parent->recurrence_edit_status = 3;
          $wrapper_parent = 
            entity_metadata_wrapper('rec_event', $entity_parent);
          // Combine parent's date with this event's time
          $new_date = $wrapper_parent->event_date->value();
          $new_date_obj = new DateObject($new_date['value']);
          $new_date['value'] = $new_date_obj->format('Y-m-d') . ' ' . 
            $date_pos->format('H:i:s');
          $new_date_obj = new DateObject($new_date['value']);
          $date_pos_end = clone $new_date_obj;
          $new_date['value2'] = 
            $date_pos_end->add($duration)->format('Y-m-d H:i:s');
          // No: undesired effects
          //RecEventController::copyFields($wrapper, $wrapper_parent);
          $wrapper_parent->event_date->set($new_date);
          // Remove edit_status from this otherwise we get infinite recursion
          // Set type=0 to avoid buildRecurrence;
          // event will be reused because the select query for existing events
          // during update does not check against this field 
          // then it gets reset by parent
          $entity->recurrence_type = 0;
          $entity->recurrence_edit_status = 0;
          rec_event_save($entity);
          // could also have called $wrapper->save();
          // Now proceed to save parent
          $wrapper_parent->save();
        }
        // if for some reason the parent no longer exists 
        // (false returned - data discrepancy)
        else {
          drupal_set_message(t(
              'Update ignored: unable to find the parent event %parent of the series for event %id', 
              array(
                  '%parent' => $entity->parent_event_id,
                  '%id' => $entity->event_id,
              )));
        }
        return $r;
      }
    }
    // is parent? YES
    else {
      // if NEW? then proceed to LOOP-THROUGH (create)
      // if EXCEPTION? return
      if ($entity->recurrence_edit_status == 1) return $r;
      // if NEXT? then set ALL
      elseif ($entity->recurrence_edit_status == 2)
        $entity->recurrence_edit_status = 3;
      // if ALL? then proceed to LOOP-THROUGH (update-create/delete)
    }
    /*
     * LOOP-THROUGH: starting values
     */
    $date_max = new DateObject(($now->format('Y') + 1) . '/12/31 23:59:00');
    //$date_max_recurring = clone $now;
    //$date_max_recurring->setTimestamp($entity->recurrence_max_date);
    if ($entity->recurrence_max_date > 0) {
      $date_max_recurring = new DateObject('@' . $entity->recurrence_max_date);
      if ($date_max_recurring < $date_max)
        $date_max = clone $date_max_recurring;
    }
    // get starting week (for weekly etc)
    $week_pos = $date_pos->format('W'); 
    $week_start = $week_pos;
    $weekly_days = isset($entity->recurrence_weekly_days)? 
      $entity->recurrence_weekly_days: array(); 
    if (!is_array($weekly_days)) $weekly_days = unserialize($weekly_days);
    // get month pos
    $month_pos = $date_pos->format('m');
    $month_start = $month_pos;
    // get occurence number
    $occ_pos = 1;
    // maximum occurences is 2 * 365
    // HC; change this and $date_max to allow more than 2 years of recurrence
    $occ_max = ($entity->recurrence_max_occ > 0)? 
      $entity->recurrence_max_occ: 730;
    // logging variables
    $msec = round(microtime(true) * 1000);
    $dates = $date_pos->format('d/m/Y H:i');
    // Get all event IDs that have parentID=this event ID by ID 
    // (we assume ID1<ID2 and date1<date2!)
    // Will be used later in updating
    $ids = array();
    // failsafe for endless recursion
    // create new events anyway if status is 0.
    if ($entity->recurrence_edit_status > 0) {
      // we dont check for recurrence_type (we assume it is ok)
      // actually it should be ok, why else a non-recurring item to have parent
      // do not change this because it will break the functionality of
      // case 'not a parent, edit all' above.
      $select = db_select('rec_event', 'e')
        ->fields('e', array('event_id'))
        ->condition('parent_event_id', $entity->event_id)
        ->orderBy('event_id');
      $ids = $select
        ->execute()
        ->fetchCol();
      // remove this ID if happens to be in array
      // shouldn't but may happen if data discrepancy
      // OR if editing subsequent, parent ID is set to this ID (was a bug)
      // If remains, problem because it gets re-saved with unforseeable results.
      $ids = array_diff($ids, array($entity->event_id));
    }
    /*
     * LOOP-THROUGH: the loop
     */
    while (($date_pos < $date_max) && ($occ_pos < $occ_max)) {
      // daily
      if ($entity->recurrence_type == 1) {
        // every x days
        if ($entity->recurrence_subtype == 0)
          $date_pos->add(new DateInterval('P' . 
              ($entity->recurrence_daily_every + 1) . 'D'));
        // every working day
        else do {
          $date_pos->add(new DateInterval('P1D'));
        } while (intval($date_pos->format('N')) > 5); // > fri
      }
      // weekly
      elseif ($entity->recurrence_type == 2) {
        do {
          $date_pos->add(new DateInterval('P1D'));
          $week_pos = $date_pos->format('W'); 
        } while (!in_array($date_pos->format('N'), $weekly_days) || // date_pos' day is not in list OR
            (($week_pos - $week_start) % ($entity->recurrence_weekly_every + 1))); // if there is modulus (correct week nums should leave mod 0)
      }
      // monthly
      elseif ($entity->recurrence_type == 3) {
        // at a specified date of every x months
        if ($entity->recurrence_subtype == 0) {
          // if month is >12 it is handled by 
          // rec_event_get_date_for_month_year()
          $month = $date_pos->format('m') + $entity->recurrence_monthly_every+1;
          $year = $date_pos->format('Y');
          $date_pos = rec_event_get_date_for_month_year(
              $entity->recurrence_at_date, $month, $year, 
              $date_pos->format('H:i:s'));
        }
        // at a specified day of a specified week of every x months
        elseif ($entity->recurrence_subtype == 1) {
          do {
            $date_pos->add(new DateInterval('P1D'));
            // get week number in month
            // http://stackoverflow.com/questions/5853380/php-get-number-of-week-for-month
            $month = $date_pos->format('m');
            $year = $date_pos->format('Y');
            $first_of_month = new DateObject($year . '/' . $month . '/1');
            $day_of_first = $first_of_month->format('N');
            $day_of_month = $date_pos->format('j');
            $week_pos = floor(($day_of_first + $day_of_month - 2) / 7) + 1;
            $day_last_of_month = $date_pos->format('t');
            $week_last = floor(($day_of_first + $day_last_of_month - 2) / 7)+1;
            $month_pos = $month;
          } while (!in_array($date_pos->format('N'), $weekly_days) || // date_pos' day is not in list OR
              (($entity->recurrence_at_week < 4) && (($entity->recurrence_at_week + 1) != $week_pos)) ||
              (($entity->recurrence_at_week == 4) && ($week_last != $week_pos)) ||
              (($month_pos - $month_start) % ($entity->recurrence_monthly_every + 1)));
        }
      }
      // yearly
      elseif ($entity->recurrence_type == 4) {
        // at a specified date of a specified month of every year
        if ($entity->recurrence_subtype == 0) {
          $month = $entity->recurrence_at_month; // 1-based
          $year = $date_pos->format('Y') + 1;
          $date_pos = rec_event_get_date_for_month_year(
              $entity->recurrence_at_date, $month, $year, 
              $date_pos->format('H:i:s'));
        }
        // at a specified day of a specified week,
        // of a specified month of every year
        elseif ($entity->recurrence_subtype == 1) {
          do {
            $date_pos->add(new DateInterval('P1D'));
            // get week number in month
            // http://stackoverflow.com/questions/5853380/php-get-number-of-week-for-month
            $month = $date_pos->format('m');
            $year = $date_pos->format('Y');
            $first_of_month = new DateObject($year . '/' . $month . '/1');
            $day_of_first = $first_of_month->format('N');
            $day_of_month = $date_pos->format('j');
            $week_pos = floor(($day_of_first + $day_of_month - 2) / 7) + 1;
            $day_last_of_month = $date_pos->format('t');
            $week_last = floor(($day_of_first + $day_last_of_month - 2) / 7)+1;
          } while (!in_array($date_pos->format('N'), $weekly_days) || // date_pos' day is not in list OR
              (($entity->recurrence_at_week < 4) && (($entity->recurrence_at_week + 1) != $week_pos)) ||
              (($entity->recurrence_at_week == 4) && ($week_last != $week_pos)) ||
              ($month != $entity->recurrence_at_month));
        }
      }
      else break; // handle THIS if in future we save anythin in parent event
      if ($date_pos > $date_max) break;
      /*
       * Update-create-delete recurring events
       */
      // get next ID (and remove from array)
      $next_id = array_shift($ids);
      if (!is_null($next_id)) {
        // get the next existing automatic recurring event
        // be careful with caching, for now we assume no reset 
        // @see rec_event_load()
        $entity_new = rec_event_load($next_id);
        // if the entity is EXCEPTION
        if ($entity_new->recurrence_edit_status == 1) {
          $wrapper_new = entity_metadata_wrapper('rec_event', $entity_new);
          // if the date is the same with date_pos then skip all together!
          $new_date = $wrapper_new->event_date->value();
          $new_date = new DateObject($new_date['value']);
          if ($date_pos->format('Y/m/d') == $new_date->format('Y/m/d')) 
            continue;
          // else get next id
          $next_id = array_shift($ids);
        }
      }
      /*
       * Update / Save new event
       * http://drupalcontrib.org/api/drupal/contributions%21commerce_coupon%21commerce_coupon.module/function/commerce_coupon_line_item_new/7
       * the following takes precedence over ::create()
       * http://www.php.net/manual/en/language.operators.array.php
       */
      if (!is_null($next_id)) $entity_new = rec_event_load($next_id);
      else $entity_new = rec_event_create(array('type' => $entity->type));
      RecEventController::copy($entity, $entity_new);
      $entity_new->recurrence_max_occ =
        ($entity->recurrence_max_occ > 0)?
        ($entity->recurrence_max_occ - $occ_pos + 1): 0;
      $entity_new->parent_event_id = $entity->event_id;
      // set new date field (Field API)
      // https://drupal.org/node/1388922
      $new_date = $parent_date;
      $new_date['value'] = $date_pos->format('Y-m-d H:i:s');
      $date_pos_end = clone $date_pos;
      $new_date['value2'] = 
        $date_pos_end->add($duration)->format('Y-m-d H:i:s');
      $wrapper_new = entity_metadata_wrapper('rec_event', $entity_new);
      // Disabled field copy. Is it necessary?
      // If enabled it would copy event registration (user ref) field too
      //RecEventController::copyFields($wrapper, $wrapper_new);
      $wrapper_new->event_date->set($new_date);
      if (isset($wrapper_new->node_reference))
        $wrapper_new->node_reference->set($wrapper->node_reference->value());
      $wrapper_new->save();
      // loop on
      $dates .= ', ' . $date_pos->format('d/m/Y H:i');
      $occ_pos++;
    } // while LOOP-THROUGH
    /*
     * here set final values in parent event (if necessary) 
     * and set $r=2 to save parent 
     * (otherwise saving has already happened in ::save)
     */
    // clear remaining IDs
    register_shutdown_function('rec_event_delete_multiple', $ids);
    watchdog('rec_event',
        'Built recurring event (%occ occurences, parent ID %id, dates %dates in %sec sec.', 
        array(
          '%occ' => $occ_pos,
          '%id' => $entity->event_id,
          '%dates' => $dates,
          '%sec' => (round(microtime(true) * 1000) - $msec) / 1000, 
        ));
    if ($r == 0) $r++;
    return $r;
  }
  
  /**
   * Copy entity primary fields
   * 
   * @param $entity
   * @param $entity_new
   */
  public static function copy($entity, &$entity_new) {
    $entity_new->title = $entity->title;
    $entity_new->description = $entity->description;
    $entity_new->recurrence_type = $entity->recurrence_type;
    $entity_new->recurrence_subtype = $entity->recurrence_subtype;
    if (isset($entity->recurrence_weekly_days))
      $entity_new->recurrence_weekly_days = $entity->recurrence_weekly_days;
    if (isset($entity->recurrence_daily_every))
      $entity_new->recurrence_daily_every = $entity->recurrence_daily_every;
    if (isset($entity->recurrence_weekly_every))
      $entity_new->recurrence_weekly_every = $entity->recurrence_weekly_every;
    if (isset($entity->recurrence_monthly_every))
      $entity_new->recurrence_monthly_every = $entity->recurrence_monthly_every;
    if (isset($entity->recurrence_at_date))
      $entity_new->recurrence_at_date = $entity->recurrence_at_date;
    if (isset($entity->recurrence_at_week))
      $entity_new->recurrence_at_week = $entity->recurrence_at_week;
    if (isset($entity->recurrence_at_month))
      $entity_new->recurrence_at_month = $entity->recurrence_at_month;
    if (isset($entity->recurrence_max_date))
      $entity_new->recurrence_max_date = $entity->recurrence_max_date;
    if (isset($entity->recurrence_max_occ))
      $entity_new->recurrence_max_occ = $entity->recurrence_max_occ;
    if (isset($entity->parent_event_id))
      $entity_new->parent_event_id = $entity->parent_event_id;
  }
  
  /**
   * Copy entity fields of Field API
   * 
   * Also works (tested) with multi-value fields.
   * 
   * This will change on D8.
   * @see https://drupal.org/node/1388922
   * @see https://api.drupal.org/api/drupal/modules!field!field.info.inc/function/field_info_instances/7
   * @see https://api.drupal.org/api/drupal/modules%21field%21field.info.inc/function/field_info_field_map/7
   * @see https://api.drupal.org/api/drupal/modules%21field%21field.info.inc/group/field_info/7
   * @see https://drupal.org/node/1021556
   * 
   * @param $wrapper
   * @param $wrapper_new
   */
  public static function copyFields($wrapper, &$wrapper_new) {
    foreach (field_info_instances('rec_event', $wrapper->getBundle()) as
        $field_name => $field_info) 
    {
      // check that indeed this field exists on destination bundle or ignore
      if (isset($wrapper_new->{$field_name}))
        $wrapper_new->{$field_name}->set($wrapper->{$field_name}->value());
    }
  }

  /**
   * Delete entity
   *
   * Disallow delete if parent event with children below
   * @see http://dev-p5qc/issues/74#comment-63
   * @see https://drupal.org/node/1848376
   * @see http://drupalcontrib.org/api/drupal/contributions!entity!includes!entity.controller.inc/function/EntityAPIController%3A%3Adelete/7
   *
   * @param $ids
   * @param DatabaseTransaction $transaction
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $entities = rec_event_load_multiple($ids);
    foreach ($entities as $key => $entity) {
      // if this is a parent event
      if (($entity->recurrence_type > 0) && is_null($entity->parent_event_id)) {
        // get number of children
        $select = db_select('rec_event', 'e')
          ->fields('e', array('event_id'))
          ->condition('parent_event_id', $entity->event_id)
          ->orderBy('event_id');
        $count = $select
          ->countQuery()
          ->execute()
          ->fetchField();
        // if there are children then do not allow
        if ($count > 0) {
          drupal_set_message(
            t('Event not deleted. This is the parent event of an event series, please edit the series so no children events remain.'),
            'error');
          $key2 = array_search($key, $ids);
          unset($ids[$key2]);
        }
      }
    }
    parent::delete($ids, $transaction);
  }

  /**
   * Build entity content output
   * 
   * 
   * @param $entity
   * @param string $view_mode
   * @param int $langcode
   * @param array $content
   * @return string
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, 
      $content = array()) 
  {
    // Get UI settings for extra fields
    $fields = field_info_extra_fields('rec_event', $entity->type, 'display');
    // foreach avoided; plain copy/paste of common field values
    // Description
    if (!empty($entity->description) && 
        $fields['description']['display'][$view_mode]['visible']) 
    {
      $content['description'] = array(
        '#theme' => 'field',
        '#title' => $fields['description']['label'],
        '#weight' => $fields['description']['display'][$view_mode]['weight'],
        '#access' => TRUE,
        '#label_display' => 'above',
        '#view_mode' => $view_mode,
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['description']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => $entity->description)),
        '#formatter' => 'text_default',
        0 => array('#markup' => check_plain($entity->description))
      );
    }
    // Date and node reference expected here (based on weight)
    // Recurrence
    $options_rec_type = _rec_event_recurrence_types();
    $recurrence = $options_rec_type[$entity->recurrence_type];
    if ($entity->recurrence_type > 0) $recurrence .= ' (';
    $options_days = rec_event_days();
    $options_months = date_month_names(TRUE);
    // Daily
    if ($entity->recurrence_type == 1) {
      if ($entity->recurrence_subtype == 0) {
        $recurrence .= format_plural($entity->recurrence_daily_every + 1, 
            'every single day', 'every @count days');
      }
      elseif ($entity->recurrence_subtype == 1)
        $recurrence .= t('every working day'); 
    }
    // Weekly
    elseif ($entity->recurrence_type == 2) {
      if ($entity->recurrence_weekly_every >= 1) {
        $recurrence .= t('every %d weeks', 
            array('%d' => $entity->recurrence_weekly_every + 1)) . ' ';
      }
      $recurrence .= t('on') . ' ';
      $first = true;
      if (isset($entity->recurrence_weekly_days)) {
        foreach (unserialize($entity->recurrence_weekly_days) as $key => $value)
        {
          if (!empty($value)) {
            if ($first) $first = false;
            else $recurrence .= ', ';
            $recurrence .= $options_days[$key];
          }
        }
      }
    }
    // Monthly
    elseif ($entity->recurrence_type == 3) {
      if ($entity->recurrence_subtype == 0) {
        if ($entity->recurrence_at_date < 31) {
          $recurrence .= t('at %date', 
              array('%date' => $entity->recurrence_at_date + 1)) . ' ';
        }
        else if ($entity->recurrence_at_date == 31) 
          $recurrence .= t('first working day') . ' ';
        else if ($entity->recurrence_at_date == 32) 
          $recurrence .= t('last working day') . ' ';
        else if ($entity->recurrence_at_date == 33) 
          $recurrence .= t('last day') . ' ';
        $recurrence .= format_plural($entity->recurrence_monthly_every + 1, 
            'of every month', 'of every @count months');
      }
      elseif ($entity->recurrence_subtype == 1) {
        if ($entity->recurrence_at_week < 4) {
          $recurrence .= t('at week %d', 
              array('%d' => $entity->recurrence_at_week + 1)) . ' ';
        }
        else $recurrence .= t('at last week') . ' ';
        $recurrence .= format_plural($entity->recurrence_monthly_every + 1, 
            'of every month', 'of every @count months') . ' ';
        $recurrence .= t('on') . ' ';
        $first = true;
        foreach (unserialize($entity->recurrence_weekly_days) as 
            $key => $value) 
        {
          if (!empty($value)) {
            if ($first) $first = false;
            else $recurrence .= ', ';
            $recurrence .= $options_days[$key];
          }
        }
      }
    }
    // Yearly
    elseif ($entity->recurrence_type == 4) {
      if ($entity->recurrence_subtype == 0) {
        if ($entity->recurrence_at_date < 31) {
          $recurrence .= t('at %date', 
              array('%date' => $entity->recurrence_at_date + 1)) . ' ';
        }
        else if ($entity->recurrence_at_date == 31) 
          $recurrence .= t('first working day') . ' ';
        else if ($entity->recurrence_at_date == 32) 
          $recurrence .= t('last working day') . ' ';
        else if ($entity->recurrence_at_date == 33) 
          $recurrence .= t('last day') . ' ';
        $recurrence .= t('of month: ');
        $recurrence .= $options_months[$entity->recurrence_at_month];
      }
      elseif ($entity->recurrence_subtype == 1) {
        if ($entity->recurrence_at_week < 4) {
          $recurrence .= t('at week %d', 
              array('%d' => $entity->recurrence_at_week + 1)) . ' ';
        }
        else $recurrence .= t('at last week') . ' ';
        $recurrence .= t('of month: ');
        $recurrence .= $options_months[$entity->recurrence_at_month] . ', ';
        $recurrence .= t('on') . ' ';
        $first = true;
        foreach (unserialize($entity->recurrence_weekly_days) as 
            $key => $value) 
        {
          if (!empty($value)) {
            if ($first) $first = false;
            else $recurrence .= ', ';
            $recurrence .= $options_days[$key];
          }
        }
      }
    }
    if (($entity->recurrence_type > 0) && $entity->recurrence_max_date) {
      $recurrence .= ', ' . t('valid until %date', 
        array('%date' => format_date($entity->recurrence_max_date, 
            'custom', 'j M Y')));
    }
    if ($entity->recurrence_max_occ > 0) {
      $recurrence .= ', ' . t('valid for less than %occ occurences',
          array('%occ' => $entity->recurrence_max_occ));
    } 
    if ($entity->recurrence_edit_status == 1)
      $recurrence .= ', ' . t('this event is an exception to the series');
    if ($entity->recurrence_type > 0) $recurrence .= ')';
    if ($fields['recurrence']['display'][$view_mode]['visible']) {
      $content['recurrence'] = array(
        '#theme' => 'field',
        '#title' => $fields['recurrence']['label'],
        '#weight' => $fields['recurrence']['display'][$view_mode]['weight'],
        '#access' => rec_event_access('edit', $entity),
        '#label_display' => 'above',
        '#view_mode' => $view_mode,
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['recurrence']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => $recurrence)),
        '#formatter' => 'text_default',
        0 => array('#markup' => $recurrence)
      );
    }
    // Created by
    $wrapper = entity_metadata_wrapper('rec_event', $entity);
    $u = $wrapper->created_uid->name->value(array('sanitize' => TRUE));
    if ($fields['author']['display'][$view_mode]['visible']) {
      $content['author'] = array(
        '#theme' => 'field',
        '#title' => $fields['author']['label'],
        '#weight' => $fields['author']['display'][$view_mode]['weight'],
        '#access' => user_access('edit any rec_event entities'),
        '#label_display' => 'above',
        '#view_mode' => $view_mode,
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['author']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => $u)),
        '#formatter' => 'text_default',
        0 => array('#markup' => l($u, 'user/' . $entity->created_uid))
      );
    }
    // Modified by
    if (($entity->created_uid != $entity->modified_uid) &&
      $fields['modifier']['display'][$view_mode]['visible'])
    {
      try {
        $u = $wrapper->modified_uid->name->value(array('sanitize' => TRUE));
      }
      catch (Exception $e) {
        $u = t('(user deleted)');
      }
      $content['modifier'] = array(
        '#theme' => 'field',
        '#title' => $fields['modifier']['label'],
        '#weight' => $fields['modifier']['display'][$view_mode]['weight'],
        '#access' => user_access('edit any rec_event entities'),
        '#label_display' => 'above',
        '#view_mode' => $view_mode,
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['modifier']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => $u)),
        '#formatter' => 'text_default',
        0 => array('#markup' => l($u, 'user/' . $entity->modified_uid))
      );
    }
    // Created date
    if ($fields['created_date']['display'][$view_mode]['visible']) {
      $content['created_date'] = array(
        '#theme' => 'field',
        '#title' => $fields['created_date']['label'],
        '#weight' => $fields['created_date']['display'][$view_mode]['weight'],
        '#access' => user_access('edit any rec_event entities'),
        '#label_display' => 'above',
        '#view_mode' => $view_mode,
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['created_date']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => format_date($entity->created_date))),
        '#formatter' => 'text_default',
        0 => array('#markup' => format_date($entity->created_date))
      );
    }
    // Modified date
    if (($entity->revision > 0) && 
        $fields['created_date']['display'][$view_mode]['visible']) 
    {
      $content['modified_date'] = array(
        '#theme' => 'field',
        '#title' => $fields['modified_date']['label'],
        '#weight' => $fields['modified_date']['display'][$view_mode]['weight'],
        '#access' => user_access('edit any rec_event entities'),
        '#label_display' => 'above',
        '#view_mode' => $view_mode,
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['modified_date']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => 
          format_date($entity->modified_date))),
        '#formatter' => 'text_default',
        0 => array('#markup' => format_date($entity->modified_date))
      );
    }
    // Revision
    if (($entity->revision > 0) && 
        $fields['created_date']['display'][$view_mode]['visible']) 
    {
      $content['revision'] = array(
        '#theme' => 'field',
        '#title' => $fields['revision']['label'],
        '#weight' => $fields['revision']['display'][$view_mode]['weight'],
        '#access' => user_access('edit any rec_event entities'),
        '#label_display' => 'above',
        '#view_mode' => 'full',
        '#language' => LANGUAGE_NONE,
        '#field_name' => $fields['revision']['label'],
        '#field_type' => 'text',
        '#entity_type' => 'rec_event',
        '#bundle' => $entity->type,
        '#items' => array(array('value' => $entity->revision)),
        '#formatter' => 'text_default',
        0 => array('#markup' => $entity->revision)
      );
    }
    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}

class RecEventTypeController 
    extends EntityAPIControllerExportable 
{
  public function create(array $values = array()) {
    $values += array(
      'label' => '',
      'description' => '',
      'state' => 0,
      'last_repop_date' => REQUEST_TIME,
    );
    return parent::create($values);
  }

  /**
   * Save entity type
   * 
   * Call save parent class and also create Field API fields.
   * Alternatively they could have been called on hook_enable()
   * Preferred here for more robustness (enable will be called less frequently).
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    rec_event_create_field_instances($entity->type);
    parent::save($entity, $transaction);
    // Rebuild menu registry. We do not call menu_rebuild directly, but set
    // variable that indicates rebuild in the end.
    // @see http://drupal.org/node/1399618
    variable_set('menu_rebuild_needed', TRUE);
  }
}

/**
 * UI controller for Event Type.
 */
class RecEventTypeUIController 
    extends EntityDefaultUIController 
{
  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = t('Manage Event types.');
    return $items;
  }
}

/**
 * Event class
 */
class RecEvent 
    extends Entity 
{
  protected function defaultLabel() {
    return $this->title;
  }

  protected function defaultUri() {
    return array('path' => 'event/' . $this->identifier());
  }
}

/**
 * Event Type class.
 */
class RecEventType 
    extends Entity 
{
  public $type;
  public $label;
  public $weight = 0;

  public function __construct($values = array()) {
    parent::__construct($values, 'rec_event_type');
  }

  function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }
}
