<?php
/**
 * @file
 *   Entity forms file
 * 
 * @author
 *   gkarak@forecasting.gr
 *   
 * @copyright
 *   (c) 2014 George Karakostas
 */

/**
 * Generates the event type editing form.
 */
function rec_event_type_form($form, &$form_state, $event_type, $op = 'edit') {
  if ($op == 'clone') {
    $event_type->label .= ' (cloned)';
    $event_type->type = '';
  }
  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $event_type->label,
    '#description' => t('The human-readable name of this type.'),
    '#required' => TRUE,
    '#size' => 30,
  );
  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($event_type->type) ? $event_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $event_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'rec_event_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this type. It must only contain lowercase letters, numbers, and underscores.'),
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => isset($event_type->description) ? $event_type->description : '',
    '#description' => t('Description about the type.'),
  );
  $form['state'] = array(
    '#type' => 'select',
    '#title' => t('Event Type State'),
    '#options' => array(
      t('Working'),
      t('Published'),
    ),
    '#default_value' => isset($event_type->state) ? $event_type->state : 0,
    '#description' => t('The state of the event types.'),
  );
  $form['_last_repop_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Repopulation Date'),
    '#size' => 20,
    '#disabled' => TRUE,
    '#default_value' => date('Y-m-d H:i:s', isset($event_type->last_repop_date) ? $event_type->last_repop_date : time()),
    '#description' => t('Last time when there has been automatic repopulation of recurring events.'),
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save type'),
    '#weight' => 40,
  );
  if (!$event_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('rec_event_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Submit handler for creating/editing event_type.
 */
function rec_event_type_form_submit(&$form, &$form_state) {
  $event_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  rec_event_type_save($event_type);

  // Redirect user back to list of types.
  $form_state['redirect'] = 'admin/structure/event-types';
}

function rec_event_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/event-types/' . $form_state['rec_event_type']->type . '/delete';
}

/**
 * Event type delete form.
 */
function rec_event_type_form_delete_confirm($form, &$form_state, $rec_event_type) {
  $form_state['event_type'] = $event_type;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['event_type_id'] = array('#type' => 'value', '#value' => entity_id('rec_event_type' ,$event_type));
  return confirm_form($form,
    t('Are you sure you want to delete type %title?', array('%title' => entity_label('rec_event_type', $event_type))),
    'event/' . entity_id('rec_event_type', $event_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Event type delete form submit handler.
 */
function rec_event_type_form_delete_confirm_submit($form, &$form_state) {
  $event_type = $form_state['event_type'];
  rec_event_type_delete($event_type);

  watchdog('rec_event_type', '@type: deleted %title.', array('@type' => $event_type->type, '%title' => $event_type->label));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => $event_type->type, '%title' => $event_type->label)));

  $form_state['redirect'] = 'admin/structure/event-types';
}

/**
 * Page to select Type to add new entity.
 * 
 * Changed for 'working' state only 
 */
function rec_event_admin_add_page() {
  $items = array();
  foreach (rec_event_types() as $event_type_key => $event_type) {
    if ($event_type->state == 0) { // working state
      $items[] = l(entity_label('rec_event_type', $event_type), 'event/add/' . $event_type_key);
      $last_event_type_key = $event_type_key;
    }
  }
  if (!count($items)) {
    return t('<p>No Event types available. Define new type at <a href="@url">Administration</a>.</p>', 
        array('@url' => '/admin/structure/event-types'));
  }
  elseif (count($items) == 1) drupal_goto('event/add/' . $last_event_type_key);
  else return array(
      'list' => array(
          '#theme' => 'item_list', 
          '#items' => $items, 
          '#title' => t('Select type to create.'),
      ),
  );
}

/**
 * Add new entity page callback.
 */
function rec_event_add($type) {
  $event_type = rec_event_types($type);

  $event = entity_create('rec_event', array('type' => $type));
  drupal_set_title(t('Create @name', array('@name' => entity_label('event_type', $event_type))));

  $output = drupal_get_form('rec_event_form', $event);

  return $output;
}

/**
 * Event Form
 */
function rec_event_form($form, &$form_state, $event) {
  $form_state['event'] = $event;
  $event_id = entity_id('rec_event', $event);
  $type = rec_event_type_load($event->type);
  $fields = field_info_extra_fields('rec_event', $event->type, 'form');
  // Title & description
  $form['title'] = array(
    '#type' => 'textfield',
    '#required' => TRUE,
    '#title' => $fields['title']['label'],
    '#default_value' => $event->title, // default value set in controller create()
    '#prefix' => '<div id="rec_event_form_ajax_ref">',
    '#suffix' => '</div>',
    '#weight' => $fields['title']['weight'],
  );
  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => $fields['description']['label'],
    '#default_value' => $event->description,
    '#weight' => $fields['description']['weight'],
  );
  // Field API fields
  field_attach_form('rec_event', $event, $form, $form_state);
  if (isset($form['node_reference'])) {
    $form['node_reference']['und']['#ajax'] = array(
      'callback' => 'rec_event_form_ajax_ref',
      'wrapper' => 'rec_event_form_ajax_ref',
    );
    // if we have form_state values (it is ajax), and if title is empty
    if (!empty($form_state['values']) && empty($form_state['values']['title']))
    {
      $e = intval($form_state
          ['values']['node_reference']['und'][0]['target_id']);
      if ($e) $form['title']['#value'] = 
          $form['node_reference']['und']['#options'][$e];
    }
  }
  // ajax for date field; would validate same dow but comes too expensive
  /*$form['event_date']['und'][0]['#ajax'] = array(
    'callback' => 'rec_event_form_ajax_ref',
    'wrapper' => 'rec_event_form_ajax_ref',
  );
  if (!empty($form_state['values']) &&
    !empty($form_state['values']['event_date']))
  {
    debug($form_state['values']['event_date']);
  }*/
  /*
   * Recurrence
   */
  $form['recurrence'] = array(
    '#title' => $fields['recurrence']['label'],
    '#prefix' => '<div id="rec_event_form_ajax_rec">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',
    '#weight' => $fields['recurrence']['weight'],
  );
  // Recurrence type
  $options_rec_type = _rec_event_recurrence_types();
  // disable editing rec type if set, because not handled in buildRecurrence()
  // @see http://dev-p5qc/issues/74#comment-63
  if (isset($event->recurrence_type) && ($event->recurrence_type > 0))
    unset($options_rec_type[0]);
  $form['recurrence']['recurrence_type'] = array(
    '#type' => 'select',
    '#title' => t('Recurrence'),
    '#options' => $options_rec_type,
    '#default_value' => 
      isset($event->recurrence_type) ? $event->recurrence_type : 0,
    '#ajax' => array(
      'callback' => 'rec_event_form_ajax_rec',
      'wrapper' => 'rec_event_form_ajax_rec',
    ),
  );
  $rec_type = isset($form_state['values']['recurrence_type'])? 
    $form_state['values']['recurrence_type']: 
    $form['recurrence']['recurrence_type']['#default_value'];
  // Recurrence subtype  
  if ($rec_type) {
    $options_rec_subtype = array(
      array(),
      array(
        t('Every day'),
        t('Every working day'),
      ),
      array(
        t('Every week'),
      ),
      array(
        t('At a specified date of every month'),
        t('At specified days of a specified week of every month'),
      ),
      array(
        t('At a specified date of a specified month of every year'),
        t('At specified days of a specified week of a specified month of every year'),
      ),
    );
    if ($rec_type != 2) { // hide for weekly
      $form['recurrence']['recurrence_subtype'] = array(
        '#type' => 'select',
        '#title' => $options_rec_type[$rec_type],
        '#options' => $options_rec_subtype[$rec_type],
        '#default_value' => 
          isset($event->recurrence_subtype) ? $event->recurrence_subtype : 0,
        '#ajax' => array(
          'callback' => 'rec_event_form_ajax_rec',
          'wrapper' => 'rec_event_form_ajax_rec',
        ),
      );
      $rec_subtype = isset($form_state['values']['recurrence_subtype'])? 
        $form_state['values']['recurrence_subtype']: 
        $form['recurrence']['recurrence_subtype']['#default_value'];
    }
    else $rec_subtype = 0;
    // get an array of numbers up to 31 to use along with array_slice
    $options_i = array();
    for ($i = 0; $i < 31; $i++)
      $options_i[$i] = $i + 1;
    array_push($options_i, t('first working day'), t('last working day'), t('last day'));
    // get 1-4 or Last
    $options_w = array_slice($options_i, 0, 4);
    array_push($options_w, t('Last'));
    // get days, months
    $options_days = rec_event_days();
    // http://drupalcontrib.org/api/drupal/contributions%21date%21date_api%21date_api.module/function/date_month_names/7
    $options_months = date_month_names(TRUE);
    // Show different fields for different type-subtype pair
    // Daily, every day
    if (($rec_type == 1) && ($rec_subtype == 0)) {
      $form['recurrence']['recurrence_daily_every'] = array(
        '#type' => 'select',
        '#title' => t('Every how many days'),
        '#options' => array_slice($options_i, 0, 6), // 1-6
        '#default_value' => isset($event->recurrence_daily_every)? 
          $event->recurrence_daily_every: 0, 
        '#description' => t('Select 1 for every day'),
      );
    }
    // Weekly
    elseif ($rec_type == 2) {
      $form['recurrence']['recurrence_weekly_every'] = array(
        '#type' => 'select',
        '#title' => t('Every how many weeks'),
        '#options' => array_slice($options_i, 0, 4), // 1-4
        '#default_value' => isset($event->recurrence_weekly_every)? 
          $event->recurrence_weekly_every: 0, 
        '#description' => t('Select 1 for every week'),
      );
      $form['recurrence']['recurrence_weekly_days'] = array(
        '#type' => 'checkboxes',
        '#title' => t('At which days'),
        '#options' => $options_days,
        '#default_value' => 
          isset($event->recurrence_weekly_days)? 
          unserialize($event->recurrence_weekly_days): 
          // get today day
          // http://stackoverflow.com/questions/712761/how-to-find-day-of-week-in-php
          //array(date('N', time()) => date('N', time()) . ''),
          array(),
      );
      // to disable a single value
      // https://drupal.org/node/342316#comment-4732130
      //$form['recurrence']['recurrence_weekly_days'][4]['#disabled'] = TRUE;
    }
    // Monthly, at a specified date of every month
    elseif (($rec_type == 3) && ($rec_subtype == 0)) {
      // show all days anyway
      $form['recurrence']['recurrence_monthly_every'] = array(
        '#type' => 'select',
        '#title' => t('Every how many months'),
        '#options' => array_slice($options_i, 0, 11), // 1-11
        '#default_value' => isset($event->recurrence_monthly_every)? 
          $event->recurrence_monthly_every: 0, 
        '#description' => t('Select 1 for every month'),
      );
      $form['recurrence']['recurrence_at_date'] = array(
        '#type' => 'select',
        '#title' => t('At which date'),
        '#options' => $options_i,
        '#default_value' => isset($event->recurrence_at_date)? 
          $event->recurrence_at_date: date('d', time()) - 1, 
      );
    }
    // Monthly, at a specified day of a specified week of every month
    elseif (($rec_type == 3) && ($rec_subtype == 1)) {
      $form['recurrence']['recurrence_monthly_every'] = array(
        '#type' => 'select',
        '#title' => t('Every how many months'),
        '#options' => array_slice($options_i, 0, 11), // 1-11
        '#default_value' => isset($event->recurrence_monthly_every)? 
          $event->recurrence_monthly_every: 0, 
        '#description' => t('Select 1 for every month'),
      );
      $form['recurrence']['recurrence_at_week'] = array(
        '#type' => 'select',
        '#title' => t('At which week'),
        '#options' => $options_w,
        '#default_value' => isset($event->recurrence_at_week)? 
          $event->recurrence_at_week: 0, 
      );
      $form['recurrence']['recurrence_weekly_days'] = array(
        '#type' => 'checkboxes',
        '#title' => t('At which days'),
        '#options' => $options_days,
        '#default_value' =>
          isset($event->recurrence_weekly_days)?
          unserialize($event->recurrence_weekly_days):
          //array(date('N', time()) => date('N', time()) . ''),
          array(),
      );
    }
      // Yearly, at a specified date of a specified month of every year
    elseif (($rec_type == 4) && ($rec_subtype == 0)) {
      $form['recurrence']['recurrence_at_month'] = array(
        '#type' => 'select',
        '#title' => t('At which month'),
        '#options' => $options_months,
        '#default_value' => isset($event->recurrence_at_month)? 
          $event->recurrence_at_month: date('n', time()), 
        '#description' => t('Select 1 for every month'),
      );
      $form['recurrence']['recurrence_at_date'] = array(
        '#type' => 'select',
        '#title' => t('At which date'),
        '#options' => $options_i,
        '#default_value' => isset($event->recurrence_at_date)? 
          $event->recurrence_at_date: date('d', time()) - 1, 
      );
    }
    // Yearly, at a specified day of a specified week of a specified month of every year
    elseif (($rec_type == 4) && ($rec_subtype == 1)) {
      $form['recurrence']['recurrence_at_month'] = array(
        '#type' => 'select',
        '#title' => t('At which month'),
        '#options' => $options_months,
        '#default_value' => isset($event->recurrence_at_month)? 
          $event->recurrence_at_month: date('n', time()), 
      );
      $form['recurrence']['recurrence_at_week'] = array(
        '#type' => 'select',
        '#title' => t('At which week'),
        '#options' => $options_w,
        '#default_value' => isset($event->recurrence_at_week)? 
          $event->recurrence_at_week: 1, 
      );
      $form['recurrence']['recurrence_weekly_days'] = array(
        '#type' => 'checkboxes',
        '#title' => t('At which days'),
        '#options' => $options_days,
        '#default_value' =>
          isset($event->recurrence_weekly_days)?
          unserialize($event->recurrence_weekly_days):
          //array(date('N', time()) => date('N', time()) . ''),
          array(),
      );
    }
    // Occurence end
    if ($rec_type > 0) {
      $form['recurrence']['recurrence_max_date'] = array(
        '#type' => 'date_popup',
        '#title' => t('Expires at'),
        '#date_format' => 'd/m/Y',
        '#default_value' => 
          isset($event->recurrence_max_date) && 
            ($event->recurrence_max_date > 0)? 
          date('Y-m-d H:i:s', $event->recurrence_max_date): '',
        // https://drupal.org/node/625264
      );
      $form['recurrence']['recurrence_max_occ'] = array(
        '#type' => 'textfield',
        '#title' => t('Maximum number of occurences'),
        '#size' => 3,
        '#default_value' => 
          (isset($event->recurrence_max_occ) && 
              ($event->recurrence_max_occ > 0))? 
          $event->recurrence_max_occ: '',
        '#description' => t('Expire after that many occurences, or before the forementioned date (whichever first).'),
        // https://api.drupal.org/api/drupal/includes%21form.inc/function/element_validate_integer_positive/7 
        '#element_validate' => array('element_validate_integer_positive'), 
      );
      // Show Edit status options if we edit & the event is recurring.
      if (!empty($event_id) && rec_event_access('edit', $event) && 
          (isset($event->recurrence_type) && $event->recurrence_type > 0)) 
      {
        $form['recurrence']['recurrence_edit_status'] = array(
            '#type' => 'select',
            '#title' => t('Edit options'),
            '#options' => array(
                1 => t('Exception to this event only'),
                2 => t('All subsequent events'),
                3 => t('All events in series'),
            ),
        );
        // Working event
        if ($type->state == 0) {
          $form['recurrence']['recurrence_edit_status']['#default_value'] = 
            (isset($event->recurrence_edit_status) && 
                ($event->recurrence_edit_status > 0))? 
            $event->recurrence_edit_status: 1;
          $form['recurrence']['recurrence_edit_status']['#description'] =
            t('Specify whether this edit applies to this event only (and all remaining recurring events remain unchanged), applies to this event and all subsequent events or to all events in the recurrence series.');
        }
        // Published event
        else {
          $form['recurrence']['recurrence_edit_status']['#disabled'] = TRUE;
          $form['recurrence']['recurrence_edit_status']['#default_value'] = 1;
          $form['recurrence']['recurrence_edit_status']['#description'] = t('This is a published event and cannot edit series recurrence.');
        }
      }
    }
  }
  // Meta fields
  $form['created_uid'] = array(
    '#type' => 'value',
    '#value' => $event->created_uid,
  );
  $form['modified_uid'] = array(
    '#type' => 'value',
    '#value' => $event->modified_uid,
  );
  $form['created_date'] = array(
    '#type' => 'value',
    '#value' => $event->created_date,
  );
  $form['modified_date'] = array(
    '#type' => 'value',
    '#value' => $event->modified_date,
  );
  // Actions
  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions'] = array(
    '#weight' => 100,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => $submit + array('rec_event_form_submit'),
  );
  // Show Delete button if we edit.
  if (!empty($event_id) && rec_event_access('edit', $event)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('rec_event_form_submit_delete'),
    );
  }
  $form['#validate'][] = 'rec_event_form_validate';
  // this will not be used, parameters will be built on submit instead
  //$form['#entity_builders'] = 'rec_event_form_submit_build_entity';
  return $form;
}

/**
 * Ajax callback for node_reference field
 * 
 * @param array $form
 * @param array $form_state
 * @return array
 */
function rec_event_form_ajax_ref($form, $form_state) {
  return $form['title'];
}

/**
 * Ajax callback for recurrence_type field
 * 
 * @param array $form
 * @param array $form_state
 * @return array
 */
function rec_event_form_ajax_rec($form, $form_state) {
  return $form['recurrence'];
}

/**
 * Form validate handler
 *
 * @param array $form
 * @param array $form_state
 */
function rec_event_form_validate($form, &$form_state) {
  // Validate only for submit button
  if (isset($form_state['values']['op']) &&
    $form_state['values']['op'] != $form_state['values']['submit']) return;
  // unnecessary: when not shown, dow is NULL
  if (isset($form_state['values']['recurrence_weekly_days'])) {
    $date = new DateObject($form_state['values']['event_date']['und'][0]['value']);
    // format N gives day of week
    if ($form_state['values']['recurrence_weekly_days'][$date->format('N')] == 0) {
      form_set_error('recurrence_weekly_days', t(
        'The day of week for the selected event date is not selected in the recurrence days of week.'));
    }
  }
}

/**
 * Event submit handler.
 * 
 * @see https://api.drupal.org/api/drupal/includes%21common.inc/function/entity_form_submit_build_entity/7
 */
function rec_event_form_submit($form, &$form_state) {
  $event = $form_state['event'];
  entity_form_submit_build_entity('rec_event', $event, $form, $form_state);
  rec_event_save($event);
  $uri = entity_uri('rec_event', $event);
  //$form_state['redirect'] = $uri['path'];
  $form_state['redirect'] = array('calendar');
  drupal_set_message(t('Event %title saved.', array('%title' => entity_label('rec_event', $event))));
}

function rec_event_form_submit_delete($form, &$form_state) {
  $event = $form_state['event'];
  $uri = entity_uri('rec_event', $event);
  $form_state['redirect'] = $uri['path'] . '/delete';
}

/**
 * Delete confirmation form.
 */
function rec_event_delete_form($form, &$form_state, $event) {
  $form_state['event'] = $event;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['event_type_id'] = array('#type' => 'value', '#value' => entity_id('rec_event' ,$event));
  $uri = entity_uri('rec_event', $event);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => entity_label('rec_event', $event))),
    $uri['path'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete form submit handler.
 */
function rec_event_delete_form_submit($form, &$form_state) {
  $event = $form_state['event'];
  drupal_set_message(t('%title submitted for deletion.', array('%title' => entity_label('rec_event', $event))));
  rec_event_delete($event);
  $form_state['redirect'] = array('calendar');
}

/**
 * Generates the publish form.
 * 
 * Form elements are not presented if no bundles available. 
 * Alternatively, they can be disabled as in batch_example.
 * 
 * @see batch_example.module
 */
function rec_event_publish_form($form, &$form_state) {
  // because it is local_task it won't get set by hook_menu
  drupal_set_title(t('Publish calendar'));
  // get bundles
  $items_working = array();
  $items_publish = array();
  foreach (rec_event_types() as $event_type_key => $event_type) {
    if ($event_type->state == 0) 
      $items_working[$event_type->type] = $event_type->label;
    else $items_publish[$event_type->type] = $event_type->label;
  }
  // build form
  $form['description'] = array(
    '#markup' => t('<p>This operation allows to publish a calendar of working type events.</p>'),
  );
  if (!count($items_working)) {
    $form['from'] = array(
      '#markup' => t('<p>No working event types available. Define new type at <a href="@url">Administration</a>.</p>', 
        array('@url' => '/admin/structure/event-types')),
    );
    return $form;
  }
  if (!count($items_publish)) {
    $form['to'] = array(
      '#markup' => t('<p>No publish event types available. Define new type at <a href="@url">Administration</a>.</p>', 
        array('@url' => '/admin/structure/event-types')),
    );
    return $form;
  }
  $form['from'] = array(
        '#type' => 'select',
        '#title' => t('Select working event types to publish from'),
        '#options' => $items_working,
  );
  $form['to'] = array(
      '#type' => 'select',
      '#title' => t('Select publish event types to publish to'),
      '#options' => $items_publish,
  );
  $options_i = array();
  for ($i = 1; $i <= 4; $i++)
    $options_i[$i] = $i;
  $options_period = array(
      'WEEK' => t('Weeks'),
      'MONTH' => t('Months'),
      'YEAR' => t('Years'),
  );
  $form['range_from'] = array(
      '#type' => 'select',
      '#title' => t('Publish from'),
      '#options' => $options_i,
      '#default_value' => 2,
  );
  $form['range_from_period'] = array(
      '#type' => 'select',
      '#options' => $options_period,
      '#default_value' => 'WEEK',
  );
  $form['range_to'] = array(
      '#type' => 'select',
      '#title' => t('Publish up to'),
      '#options' => $options_i,
      '#default_value' => 2,
  );
  $form['range_to_period'] = array(
      '#type' => 'select',
      '#options' => $options_period,
      '#default_value' => 'MONTH',
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Publish calendar'),
      '#weight' => 40,
  );
  return $form;
}

/**
 * Publish form submit handler.
 * 
 * @see batch_example.module
 * @see https://drupal.org/node/180528
 * @see https://api.drupal.org/api/drupal/includes%21form.inc/group/batch/7
 */
function rec_event_publish_form_submit($form, &$form_state) {
  /*SELECT e.event_id, e2.event_id
  FROM rec_event e
  INNER JOIN field_data_event_date d ON e.event_id = d.entity_id AND e.type = d.bundle
  LEFT JOIN rec_event e2 ON e.event_id = e2.ref_event_id
  WHERE d.entity_type = 'rec_event'
  AND d.bundle = 'working'
  AND d.event_date_value > DATE_SUB(NOW(), INTERVAL 2 WEEK)
  AND d.event_date_value < DATE_ADD(NOW(), INTERVAL 2 WEEK)
  ORDER BY e.event_id */
  $query = db_select('rec_event', 'e');
  $query->join('field_data_event_date', 'd', 
      'e.event_id = d.entity_id AND e.type = d.bundle');
  $query->leftJoin('rec_event', 'e2', 'e.event_id = e2.ref_event_id');
  $query->addField('e', 'event_id');
  $query->addField('e2', 'event_id', 'publish_id');
  $query
    ->condition('d.entity_type', 'rec_event')
    ->condition('d.bundle', $form_state['values']['from'])
    ->where('d.event_date_value > DATE_SUB(NOW(), INTERVAL ' . 
        $form_state['values']['range_from'] . ' ' . 
        $form_state['values']['range_from_period'] . ')')
    ->where('d.event_date_value < DATE_ADD(NOW(), INTERVAL ' . 
        $form_state['values']['range_to'] . ' ' . 
        $form_state['values']['range_to_period'] . ')')
    ->orderBy('e.event_id');
  $ids = $query
    ->execute()
    ->fetchAll();
  $count = count($ids);
  $operations = array();
  foreach ($ids as $event) {
    $operations[] = array(
        'rec_event_publish_process',
        array(
            $event->event_id, 
            $event->publish_id, 
            $count, 
            $form_state['values']['to'],
        ),
    );
  }
  batch_set(array(
      'operations' => $operations,
      'finished' => 'rec_event_publish_finished',
  ));
  $form_state['redirect'] = 'event';
}
